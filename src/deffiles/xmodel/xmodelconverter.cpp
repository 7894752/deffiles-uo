/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#include <windows.h>
#include "../convert_value.h"
#include "xmodel_model.h"
#include "shaderlib.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

#pragma intrinsic(memset, memcpy)

enum {
	MODELTYPE_RIGID,
	MODELTYPE_ANIMATED,
	MODELTYPE_VIEWMODEL,
	MODELTYPE_MULTIPLAYER_BODY,
	MODELTYPE_VIEWHANDS,
	MODELTYPE_COUNT
};

extern "C" {
char *va( const char *fmt, ... );
void Com_BeginParseSession( const char *name );
void Com_EndParseSession();
void Com_SetCSV( int csv );
char *Com_Parse( const char **text );
int MyAssertHandler( const char *expression, const char *file, int line , ...);
char *COM_Parse( char *text );
extern char com_token[1024];

}
int FS_ReadFile( const char *name, void **buffer );
extern "C++" void Error( const char *fmt, ... );

// Hit location names stored with model bones.
static const char *hitLocationNames[] = {
	"none", "helmet", "head", "neck", "torso_upper", "torso_lower",
	"right_arm_upper", "left_arm_upper", "right_arm_lower", "left_arm_lower",
	"right_hand", "left_hand", "right_leg_upper", "left_leg_upper",
	"right_leg_lower", "left_leg_lower", "right_foot", "left_foot", "gun"
};

static bool classificationsLoaded[2] = {false, false};
PartClassification partClassifications[2][MAX_CLASSIFICATION] = {0};
loadcollmodel_t collmodel = {0};
int numPartClassifications[2] = {0, 0};
ConvertValue *convertValue = 0;

static void InitPartClassification( ConvertValue *cv, int classification, const char *sourceFile );

/*
=============
DllMain
=============
*/
extern "C" int __stdcall DllMain( void *instance, unsigned long reason, void *reserved ) {
	switch ( reason ) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return 1;
}

/*
=============
ExtractFileBase
=============
*/
static void ExtractFileBase( const char *path, char *dest ) {
	const char *p = path + strlen( path ) - 1;
	for ( ; p >= path; p-- ) {
		if ( *p == '/' || *p == '\\' ) {
			strcpy( dest, p + 1 );
			return;
		}
	}
	*dest = 0;
}

/*
=============
StripExtension
=============
*/
static void StripExtension( char *path ) {
	char *p = path + strlen( path ) - 1;
	for ( ; p >= path; p-- ) {
		if ( *p == '.' ) {
			*p = 0;
			return;
		}
	}
}

int FS_ReadFile(const char *name, void **buffer) {
	FILE *file;
	int len;
	char *data;
	file = fopen(name, "rb");
	if (!file) return -1;
	fseek(file, 0, SEEK_END);
	len = ftell(file);
	fseek(file, 0, SEEK_SET);
	data = (char *)malloc(len + 1);
	if (fread(data, 1, len, file) != len) Com_Error(0, "Short read in FS_ReadFile()\n");
	data[len] = 0;
	fclose(file);
	*buffer = data;
	return len;
}

/*
=============
GetHitLocation
=============
*/
static int GetHitLocation( const char *name ) {
	for ( unsigned int i = 0; i < sizeof( hitLocationNames ) / sizeof( hitLocationNames[0] ); i++ ) {
		if ( !stricmp( name, hitLocationNames[i] ) ) return i;
	}
	throw va( "unknown hit location '%s'", name );
}

/*
=============
InitPartClassification
=============
*/
void InitPartClassification( ConvertValue &cv ) {
	InitPartClassification( &cv, 0, "source_data/partclassification.csv" );
	InitPartClassification( &cv, 1, "source_data/partclassification_mp.csv" );
}

static void InitPartClassification( ConvertValue *cv, int classification, const char *sourceFile ) {
	char pszFileName[1024];
	char *file;
	const char *ptr;
	char *token;
	sprintf( pszFileName, "%s%s", cv->GetSourceDirectory(), sourceFile );
	cv->AddSourceFile( pszFileName );
	if ( classificationsLoaded[classification] ) return;
	classificationsLoaded[classification] = true;
	if ( FS_ReadFile( pszFileName, (void **)&file ) < 0 ) throw va( "could not find '%s'", pszFileName );
	Com_BeginParseSession( pszFileName );
	Com_SetCSV( 1 );
	ptr = file;
	for ( ;; ) {
		if ( numPartClassifications[classification] >= MAX_CLASSIFICATION ) throw "MAX_CLASSIFICATION exceeded";
		token = Com_Parse( &ptr );
		if ( !ptr ) break;
		strcpy( partClassifications[classification][numPartClassifications[classification]].name, token );
		token = Com_Parse( &ptr );
		if ( !ptr ) {
			Com_EndParseSession();
			free( file );
			throw va( "error in reading '%s'", pszFileName );
		}
		partClassifications[classification][numPartClassifications[classification]].classification = GetHitLocation( token );
		numPartClassifications[classification]++;
	}
	Com_EndParseSession();
	free( file );
}


extern void BuildModel( xmodelInfo_t *lods, int lod, const char *source, const char *parts,
	const char *sourceDir, const char *gameDir, const char *skinDir,
	bool writeOffsets, const char *surfs, const char *controllers, const char *stabilizers,
	modelBoneNames_t *bones, bool radialNormals, const float radialOrigin[3], int animated,
	const char *rigidRoot, skinNames_t *surfNames, skinOverride_t overrides[], int overrideCount,
	bool viewmodel, const char *hitBox, int classification );

/*
=============
ConvertModelLod
=============
*/
static void ConvertModelLod( ConvertValue *cv, xmodelInfo_t *lods, int lod, bool writeParts,
	const char *controllers, const char *stabilizers, modelBoneNames_t *bones, bool radialNormals,
	const float *radialOrigin, bool writeOffsets, int type, const char *rigidRoot,
	skinNames_t *surfNames, skinOverride_t *overrides, int overrideCount, const char *hitBox )
{
	char srcSkinDir[1024];
	char destRootDir[1024];
	char destRelSkinDir[1024];
	char destDataDir[1024];
	char destSurfDir[1024];

	char *p;
	char srcModelFilename[1024];
	char destModelFilename[1024];
	char destSurfFilename[1024];
	char shortName[1024];

	char *filename = lods->lods[lod].filename;
	int animated;
	bool viewmodel;
	char hitBoxModelFilename[1024];
	int classification;

	assert( filename[0] );
	sprintf( srcModelFilename, "%smodel_export/%s", cv->GetSourceDirectory(), filename );
	strcpy( destDataDir, cv->GetTargetDirectory( 2 ) );
	ExtractFileBase( filename, shortName );
	StripExtension( shortName );
	strcpy( filename, shortName );
	strcat( filename, va( "%i", type ) );
	sprintf( destModelFilename, "%s%s", destDataDir, filename );
	strcpy( srcSkinDir, srcModelFilename );
	for ( p = srcSkinDir + strlen( srcSkinDir ) - 1; ; p-- ) {
		if ( *p == '/' || *p == '\\' ) {
			p++;
			*p = 0;
			break;
		}
	}
	shaderLibrary.cv = cv;
	sprintf( shaderLibrary.dir, "%s%s", cv->GetSourceRootDirectory(), cv->GetTargetRootDirectory() );
	sprintf( shaderLibrary.gameDir, "%s/game/main/", cv->GetSourceRootDirectory() );
	sprintf( destRootDir, "%s%s", cv->GetSourceRootDirectory(), cv->GetTargetRootDirectory() );
	strcpy( destRelSkinDir, cv->GetRelativeTargetDirectory( 1 ) );
	strcpy( destSurfDir, cv->GetTargetDirectory( 3 ) );
	sprintf( destSurfFilename, "%s%s", destSurfDir, filename );
	printf( "Converting xmodel '%s'...\n", srcModelFilename );
	cv->AddSourceFile( srcModelFilename );
	if ( writeParts ) cv->AddTargetFile( destModelFilename );
	cv->AddTargetFile( destSurfFilename );
	convertValue = cv;
	animated = type != MODELTYPE_RIGID;
	viewmodel = type == MODELTYPE_VIEWHANDS;
	classification = type == MODELTYPE_MULTIPLAYER_BODY;
	if ( writeParts && hitBox[0] ) {
		sprintf( hitBoxModelFilename, "%smodel_export/%s", cv->GetSourceDirectory(), hitBox );
		cv->AddSourceFile( hitBoxModelFilename );
	} else {
		hitBoxModelFilename[0] = 0;
	}
	BuildModel( lods, lod, srcModelFilename, writeParts ? destModelFilename : NULL,
		srcSkinDir, destRootDir, destRelSkinDir, writeOffsets, destSurfFilename,
		controllers, stabilizers, bones, radialNormals, radialOrigin, animated,
		rigidRoot, surfNames, overrides, overrideCount, viewmodel, hitBoxModelFilename, classification );
}

/*
=============
ParseSkinOverrides
=============
Read material-to-image overrides from the converter input string.
Each override begins with the material name from the model export.
An optional parenthesized object name narrows the affected surfaces.
The following skin name supplies the replacement image.
Retain the input order so later surface resolution sees the same list.
New entries begin unused and are marked when a surface consumes them.
Reject incomplete records and excess entries with a source diagnostic.
Return the number of complete override records written to the array.
*/
static int ParseSkinOverrides(skinOverride_t *overrides, char *skinText)
{
	int count = 0;
	char *text;
	for (text = COM_Parse(skinText); text; text = COM_Parse(text))
	{
		if (count >= MAX_SKIN_OVERRIDE)
			Error("MAX_SKIN_OVERRIDE exceeded");
		memset(overrides[count].material, 0, 64);
		strncpy(overrides[count].material, com_token, 63);
		overrides[count].used = false;
		text = COM_Parse(text);
		if (!text)
			Error("no skin override specified for %s",
				  overrides[count].material);
		memset(overrides[count].object, 0, 64);
		if (!strcmp(com_token, "("))
		{
			text = COM_Parse(text);
			if (!text)
				Error("no object for skin override specified for %s",
					  overrides[count].material);
			strncpy(overrides[count].object, com_token, 63);
			text = COM_Parse(text);
			if (!text || strcmp(com_token, ")"))
				Error("expecting ')' for skin override of '%s'",
					  overrides[count].material);
			text = COM_Parse(text);
			if (!text)
				Error("no skin override specified for %s",
					  overrides[count].material);
		}
		memset(overrides[count].skin, 0, 64);
		strncpy(overrides[count].skin, com_token, 63);
		count++;
	}
	return count;
}

/*
=============
FreeSurfaceNames
=============
Release the copied surface names accumulated for the three detail levels.
Each list owns its duplicated strings, while the list storage remains.
Reset every count after releasing the strings.
The converter can then reuse the same lists for another model.
*/
static void FreeSurfaceNames(skinNames_t *lists)
{
	for (int lod = 0; lod < 3; lod++)
	{
		for (int i = 0; i < lists[lod].num; i++)
			free(lists[lod].names[i]);
		lists[lod].num = 0;
	}
}

extern loadcollmodel_t collmodel;
extern "C" void SaveXModel(const char *filename, xmodelInfo_t *lods,
						   skinNames_t *surfNames);
extern "C" void Com_Printf(const char *fmt, ...);

// User-visible model categories follow the converter model-type enumeration.
// Keep this order aligned with the runtime conversion flags.
// These names identify the selected variant in conversion diagnostics.
static const char *modelTypeNames[] = {
	"rigid",
	"animated",
	"viewmodel",
	"multiplayer body",
	"viewhands"
};

/*
=============
Convert
=============
*/
extern "C" __declspec(dllexport) int Convert( ConvertValue *cv ) {
	xmodelInfo_t config;
	char configFilename[1024];
	char configTargetDir[1024];
	char configCollLOD[1024];
	int i;
	int j;
	int calculateBounds;
	modelBoneNames_t *baseHierarchy;
	modelBoneNames_t *hierarchy;
	char boneControllers[2048];
	char boneStabilizers[2048];
	char skinOverrideString[4096];
	bool radialNormals;
	float radialOrigin[3];
	const char *typeName;
	int iModelType[3];
	char rigidRootName[3][1024];
	bool writeOffsets;
	char *file;
	const char *ptr;
	char *token;
	char pszFileName[1024];
	bool forceRigid;
	skinNames_t surfNames[3];
	skinOverride_t skinOverride[MAX_SKIN_OVERRIDE];
	int overrideCount;
	char hitBoxModel[1024];

	try {
		for ( i = 0; i < 3; i++ ) {
			surfNames[i].num = 0;
		}
		InitPartClassification( *cv );

		baseHierarchy = new modelBoneNames_t();
		hierarchy = new modelBoneNames_t();

		strcpy( config.lods[0].filename, cv->GetValue( "filename" )->GetString() );
		strcpy( config.lods[1].filename, cv->GetValue( "mediumLod" )->GetString() );
		strcpy( config.lods[2].filename, cv->GetValue( "lowLod" )->GetString() );
		
		typeName = cv->GetValue( "type" )->GetString();
		for ( iModelType[0] = MODELTYPE_RIGID; iModelType[0] < MODELTYPE_COUNT; iModelType[0]++ ) {
			if ( !stricmp( modelTypeNames[iModelType[0]], typeName ) ) {
				break;
			}
		}
		
		if ( iModelType[0] == MODELTYPE_COUNT ) {
			throw "bad type";
		}

		strcpy( rigidRootName[0], cv->GetValue( "highLodRigidRoot" )->GetString() );
		
		forceRigid = cv->GetValue( "forceMediumLodRigid" )->GetInteger() != 0;
		iModelType[1] = forceRigid ? MODELTYPE_RIGID : iModelType[0];
		strcpy( rigidRootName[1], cv->GetValue( "mediumLodRigidRoot" )->GetString() );
		if ( iModelType[0] == MODELTYPE_RIGID ) {
			assert( iModelType[1] == MODELTYPE_RIGID );

			if ( stricmp( rigidRootName[1], rigidRootName[0] ) ) {
				if ( config.lods[1].filename[0] ) {
					Com_Printf( "*** WARNING: Overriding mediumLodRigidRoot '%s' with '%s'\n",
						rigidRootName[1], rigidRootName[0] );
				}
				strcpy( rigidRootName[1], rigidRootName[0] );
			}
		}
		
		forceRigid = cv->GetValue( "forceLowLodRigid" )->GetInteger() != 0;
		iModelType[2] = forceRigid ? MODELTYPE_RIGID : iModelType[1];
		strcpy( rigidRootName[2], cv->GetValue( "lowLodRigidRoot" )->GetString() );
		if ( iModelType[1] == MODELTYPE_RIGID ) {
			assert( iModelType[2] == MODELTYPE_RIGID );
			if ( stricmp( rigidRootName[2], rigidRootName[1] ) ) {
				if ( config.lods[2].filename[0] ) Com_Printf( "*** WARNING: Overriding lowLodRigidRoot '%s' with '%s'\n", rigidRootName[2], rigidRootName[1] );
				strcpy( rigidRootName[2], rigidRootName[1] );
			}
		}
		writeOffsets = iModelType[0] != MODELTYPE_VIEWMODEL && iModelType[0] != MODELTYPE_VIEWHANDS;
		config.lods[0].distance = cv->GetValue( "highLodDist" )->GetFloat();
		config.lods[1].distance = cv->GetValue( "mediumLodDist" )->GetFloat();
		config.lods[2].distance = cv->GetValue( "lowLodDist" )->GetFloat();
		calculateBounds = cv->GetValue( "calculateBounds" )->GetInteger();
		if ( calculateBounds ) {
			config.mins[0] = 1; config.mins[1] = 1; config.mins[2] = 1;
			config.maxs[0] = -1; config.maxs[1] = -1; config.maxs[2] = -1;
		} else {
			config.mins[0] = cv->GetValue( "minX" )->GetFloat();
			config.mins[1] = cv->GetValue( "minY" )->GetFloat();
			config.mins[2] = cv->GetValue( "minZ" )->GetFloat();
			config.maxs[0] = cv->GetValue( "maxX" )->GetFloat();
			config.maxs[1] = cv->GetValue( "maxY" )->GetFloat();
			config.maxs[2] = cv->GetValue( "maxZ" )->GetFloat();
		}
		strcpy( configCollLOD, cv->GetValue( "collisionLOD" )->GetString() );
		config.collisionLod = -1;
		memset( &collmodel, 0, sizeof( collmodel ) );
		if ( !strcmp( configCollLOD, "High" ) ) config.collisionLod = 0;
		else if ( !strcmp( configCollLOD, "Medium" ) ) config.collisionLod = 1;
		else if ( !strcmp( configCollLOD, "Low" ) ) config.collisionLod = 2;
		radialNormals = cv->GetValue( "noRadialNormals" )->GetInteger() == 0;
		if ( radialNormals ) {
			radialOrigin[0] = cv->GetValue( "radialOriginX" )->GetFloat();
			radialOrigin[1] = cv->GetValue( "radialOriginY" )->GetFloat();
			radialOrigin[2] = cv->GetValue( "radialOriginZ" )->GetFloat();
		}
		strcpy( boneControllers, cv->GetValue( "boneControllers" )->GetString() );
		strcpy( boneStabilizers, cv->GetValue( "boneStabilizers" )->GetString() );
		strcpy( skinOverrideString, cv->GetValue( "skinOverride" )->GetString() );
		if ( iModelType[0] == MODELTYPE_MULTIPLAYER_BODY ) {
			sprintf( pszFileName, "%s%s", cv->GetSourceDirectory(), "source_data/multiplayercontrollers.csv" );
			cv->AddSourceFile( pszFileName );
			if ( FS_ReadFile( pszFileName, (void **)&file ) < 0 ) throw va( "could not find '%s'", pszFileName );
			Com_BeginParseSession( pszFileName );
			Com_SetCSV( 1 );
			ptr = file;
			for ( ;; ) {
				token = Com_Parse( &ptr );
				if ( !ptr ) break;
				strcat( boneControllers, va( " \"%s\"", token ) );
				token = Com_Parse( &ptr );
				if ( !ptr ) { Com_EndParseSession(); free( file ); throw va( "error in reading '%s'", pszFileName ); }
				strcat( boneControllers, va( " \"%s\"", token ) );
			}
			Com_EndParseSession(); free( file );
			sprintf( pszFileName, "%s%s", cv->GetSourceDirectory(), "source_data/multiplayerstabilizers.csv" );
			cv->AddSourceFile( pszFileName );
			if ( FS_ReadFile( pszFileName, (void **)&file ) < 0 ) throw va( "could not find '%s'", pszFileName );
			Com_BeginParseSession( pszFileName ); Com_SetCSV( 1 ); ptr = file;
			for ( ;; ) {
				token = Com_Parse( &ptr );
				if ( !ptr ) break;
				strcat( boneStabilizers, va( " \"%s\"", token ) );
				token = Com_Parse( &ptr );
				if ( !ptr ) { Com_EndParseSession(); free( file ); throw va( "error in reading '%s'", pszFileName ); }
				strcat( boneStabilizers, va( " \"%s\"", token ) );
			}
			Com_EndParseSession(); free( file );
		}
		strcpy( configTargetDir, cv->GetTargetDirectory( 0 ) );
		sprintf( configFilename, "%s%s", configTargetDir, cv->GetName() );
		cv->AddTargetFile( configFilename );
		if ( !config.lods[0].filename[0] ) throw "no filename specified";
		for ( i = 0; i < 3; i++ ) {
			if ( config.lods[i].distance < 0 ) throw "negative LOD distance not allowed";
			if ( config.lods[i].distance != 0 ) {
				if ( !config.lods[i].filename[0] ) throw "LOD distance without a filename is not allowed";
			}
			if ( !config.lods[i].filename[0] && config.collisionLod >= i ) throw "collision LOD is not valid";
		}
		overrideCount = ParseSkinOverrides( skinOverride, skinOverrideString );
		strcpy( hitBoxModel, cv->GetValue( "hitBoxModel" )->GetString() );
		ConvertModelLod( cv, &config, 0, true, boneControllers, boneStabilizers, baseHierarchy, radialNormals,
			radialOrigin, writeOffsets, iModelType[0], rigidRootName[0], &surfNames[0], skinOverride, overrideCount, hitBoxModel );
		for ( i = 1; i < 3; i++ ) {
			if ( !config.lods[i].filename[0] ) continue;
			if ( !config.lods[i-1].filename[0] ) throw va( "missing LOD above '%s'", config.lods[i].filename );
			if ( (config.lods[i].distance != 0 && config.lods[i].distance <= config.lods[i-1].distance) || config.lods[i-1].distance == 0 )
				throw va( "LOD '%s' does not have greater distance than LOD '%s'", config.lods[i].filename, config.lods[i-1].filename );
			ConvertModelLod( cv, &config, i, false, boneControllers, boneStabilizers, hierarchy, radialNormals,
				radialOrigin, writeOffsets, iModelType[i], rigidRootName[i], &surfNames[i], skinOverride, overrideCount, NULL );
			if ( hierarchy->count > baseHierarchy->count ) throw va( "Lower lod '%s' has more parts than '%s'", config.lods[i].filename, config.lods[0].filename );
			for ( j = 1; j < hierarchy->count; j++ ) {
				if ( stricmp( hierarchy->names[j], baseHierarchy->names[j] ) ) throw va( "Part '%s' in lower lod '%s' does not have the same name as the part '%s' in higher lod '%s'", hierarchy->names[j], config.lods[i].filename, baseHierarchy->names[j], config.lods[0].filename );
				if ( hierarchy->parents[j] != baseHierarchy->parents[j] ) throw va( "Part '%s' (with parent '%s') in lower lod '%s' does not have the same parent as the matching part (with parent '%s') in higher lod '%s'", config.lods[i].filename, hierarchy->parents[j] != -1 ? hierarchy->names[hierarchy->parents[j]] : "<none>", hierarchy->names[j], baseHierarchy->parents[j] != -1 ? baseHierarchy->names[baseHierarchy->parents[j]] : "<none>", config.lods[0].filename );
			}
		}
		for ( i = 0; i < overrideCount; i++ ) {
			if ( skinOverride[i].used ) continue;
			Error( "original skin '%s' (object '%s') specified by skinOverride not used", skinOverride[i].material, skinOverride[i].object );
		}
		SaveXModel( configFilename, &config, surfNames );
		delete baseHierarchy; delete hierarchy;
		FreeSurfaceNames( surfNames );
		fflush( stdout );
		return 0;
	} catch ( char *error ) {
		printf( "\n************ ERROR ************\n" );
		printf( "%s\n", error );
		delete baseHierarchy; delete hierarchy;
		FreeSurfaceNames( surfNames );
		fflush( stdout );
		return 1;
	}
}
