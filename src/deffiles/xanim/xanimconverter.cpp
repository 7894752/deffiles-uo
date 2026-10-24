/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Asset-manager entry point and model-match tables.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdafx.h"
#include "../convert_value.h"
#include "xanim_model.h"

extern "C" void Com_BeginParseSession( const char *filename );
extern "C" void Com_EndParseSession();
extern "C" void Com_SetCSV( int enabled );
extern "C" char *Com_Parse( char **text );
extern "C" char *Com_ParseOnLine( char **text );
extern "C" void Com_SkipRestOfLine( char **text );
int FS_ReadFile( const char *filename, void **buffer );
extern "C" char *va( const char *format, ... );

static AnimConvertInfo options = { 0 };
static bool modelMatchesLoaded = false;
extern "C" int conversionError;

extern "C" void Com_Error(int code, const char *fmt, ...);

#include "files.h"

/*
=============
FS_ReadFile
=============
*/
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
ReadModelMatchList
=============
*/
void ReadModelMatchList( ConvertValue *cv ) {
	char pszFileName[1024];
	void *file;
	char *ptr;
	char *name;
	bool first;
	sprintf( pszFileName, "%s%s", cv->GetSourceDirectory(), "source_data/modelmatch.csv" );
	cv->AddSourceFile( pszFileName );
	if ( modelMatchesLoaded ) return;
	modelMatchesLoaded = true;
	if ( FS_ReadFile( pszFileName, &file ) < 0 ) {
		throw va( "could not find '%s'", pszFileName );
	}
	Com_BeginParseSession( pszFileName );
	Com_SetCSV( true );
	first = true;
	ptr = (char *)file;
	for ( ;; ) {
		name = Com_ParseOnLine( &ptr );
		if ( !ptr ) break;
		if ( !name[0] ) {
			Com_SkipRestOfLine( &ptr );
			first = true;
			continue;
		}
		sprintf( pszFileName, "%smodel_export/%s", cv->GetSourceDirectory(), name );
		try {
			AddQuatBoneList( pszFileName, first );
		} catch ( char *error ) {
			char error2[1024];
			Com_EndParseSession();
			free( file );
			sprintf( error2, "Error in '%s':\n%s", "source_data/modelmatch.csv", error );
			throw error2;
		}
		first = false;
	}
	Com_EndParseSession();
	free( file );
}
/*
=============
Convert
=============
*/
extern "C" __declspec(dllexport) int Convert( ConvertValue *cv ) {
	char type_string[1024];
	const char *modelName = 0;
	const char *animName;
	char *file;
	char *ptr;
	char *bone;
	char pszFileName[1024];
	try {
		animName = cv->GetValue( "filename" )->GetString();
		if ( !animName[0] ) Error( "filename was not specified" );
		sprintf( options.animFilename, "%sxanim_export/%s", cv->GetSourceDirectory(), animName );
		printf( "Converting xanim '%s'...\n", options.animFilename );
		sprintf( options.outputFilename, "%s%s", cv->GetTargetDirectory( 0 ), cv->GetName() );
		options.looping = cv->GetValue( "looping" )->GetInteger() != 0;
		options.useBones = cv->GetValue( "useBones" )->GetInteger() != 0;
		strcpy( type_string, cv->GetValue( "type" )->GetString() );
		if ( !stricmp( type_string, "absolute" ) ) options.type = 0;
		else if ( !stricmp( type_string, "relative" ) ) options.type = 1;
		else if ( !stricmp( type_string, "delta" ) ) options.type = 2;
		else if ( !stricmp( type_string, "mp_torso" ) ) options.type = 3;
		else if ( !stricmp( type_string, "mp_legs" ) ) options.type = 4;
		else if ( !stricmp( type_string, "mp_fullbody" ) ) options.type = 5;
		else Error( "bad anim type '%s' for '%s'", type_string, cv->GetName() );
		strcpy( options.node, cv->GetValue( "node" )->GetString() );
		options.rotationThreshold = cv->GetValue( "angleError" )->GetFloat();
		if ( options.rotationThreshold < 0 ) Error( "negative angleError not allowed" );
		options.translationThreshold = cv->GetValue( "translationError" )->GetFloat();
		if ( options.translationThreshold < 0 ) Error( "negative translationError not allowed" );
		strcpy( options.stabilizers, cv->GetValue( "boneStabilizers" )->GetString() );
		if ( options.type == 3 || options.type == 4 || options.type == 5 ) {
			if ( options.type == 3 || options.type == 5 ) {
				sprintf( pszFileName, "%s%s", cv->GetSourceDirectory(), "source_data/multiplayerstabilizers.csv" );
				cv->AddSourceFile( pszFileName );
				if ( FS_ReadFile( pszFileName, (void **)&file ) < 0 ) {
					throw va( "could not find '%s'", pszFileName );
				}
				Com_BeginParseSession( pszFileName );
				Com_SetCSV( true );
				ptr = file;
				for ( ;; ) {
					bone = Com_Parse( &ptr );
					if ( !ptr ) break;
					strcat( options.stabilizers, va( " \"%s\"", bone ) );
					bone = Com_Parse( &ptr );
					if ( !ptr ) {
						Com_EndParseSession();
						free( file );
						throw va( "error in reading '%s'", pszFileName );
					}
					strcat( options.stabilizers, va( " \"%s\"", bone ) );
				}
				Com_EndParseSession();
				free( file );
			}
			switch ( options.type ) {
			case 3: modelName = "IW\\character_models\\mp\\torso.XMODEL_EXPORT"; options.type = 1; break;
			case 4: modelName = "IW\\character_models\\mp\\legs.XMODEL_EXPORT"; options.type = 1; break;
			case 5: modelName = "IW\\character_models\\mp\\fullbody.XMODEL_EXPORT"; options.type = 2; break;
			}
		}
		if ( !modelName ) {
			modelName = cv->GetValue( "model" )->GetString();
			if ( !modelName[0] ) Error( "model was not specified" );
		}
		sprintf( options.modelFilename, "%smodel_export/%s", cv->GetSourceDirectory(), modelName );
		cv->AddSourceFile( options.modelFilename );
		cv->AddSourceFile( options.animFilename );
		cv->AddTargetFile( options.outputFilename );
		ReadModelMatchList( cv );
		ConvertAnimation( &options );
		fflush( stdout );
		return 0;
	} catch ( char *error ) {
		printf( "\n************ ERROR ************\n" );
		printf( "%s\n", error );
		fflush( stdout );
		return 1;
	}
}
