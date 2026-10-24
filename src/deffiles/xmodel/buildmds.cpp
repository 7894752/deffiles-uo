/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/
#ifndef XANIM_CONVERTER
#include "modelbuild.h"
#else
// Model and animation export loading, hierarchy building and surface setup.

#include "animation_types.h"
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <malloc.h>
#include <ctype.h>
#include "animation_model.h"
#include <math.h>
#include <float.h>
#include "q_round.h"

#pragma intrinsic(memset, memcpy)
#endif
#ifndef XANIM_CONVERTER
ShaderInfo shaderLibrary;

/*
=============
SkelModel::SkelModel
=============
*/
SkelModel::SkelModel() {
	InitModel();
}

/*
=============
SkelModel::~SkelModel
=============
*/
SkelModel::~SkelModel() {
	FreeModel();
}

/*
=============
SkelModel::InitModel
=============
*/
void SkelModel::InitModel() {
	memset( &model, 0, sizeof( model ) );
	numControllers = 0;
	numStabilizers = 0;
}

/*
=============
SkelModel::FreeModel
=============
*/
void SkelModel::FreeModel() {
	int i;

	for ( i = 0; i < model.numframes; i++ ) {
		delete[] model.poses[i].pose;
	}
	delete[] model.nodeFrames;
	delete[] model.vertRemap;
	delete[] model.verts;
	delete[] model.vertsSaved;
	delete[] model.faces;
	delete[] model.poses;
	delete[] model.bones;
	delete[] model.boneRemap;
	delete[] model.surfaces;
	delete[] model.objectNames;
	delete[] model.materialNames;
	delete[] model.animToModel;
	memset( &model, 0, sizeof( model ) );
}

/*
=============
Model::ParseBaseFile
=============
*/
void Model::ParseBaseFile( char filename[] ) {
	Script script;
	script.LoadFile( filename );
	script.Skip( "MODEL", true );
	script.Skip( "VERSION", true );
	int version = script.GetInteger( false );
	if ( version != 5 ) {
		script.Error( "Expecting version %d but found version %d.\n", 5, version );
	}
	LoadBoneSection( &script );
	ReadBaseBones( &script );
}

/*
=============
SkelModel::SetBoneControllers
=============
*/
void SkelModel::SetBoneControllers( const char *input ) {
	const char *text;
	numControllers = 0;
	for ( text = COM_Parse( input ); text; text = COM_Parse( text ) ) {
		if ( numControllers >= 8 ) {
			Error( "maximum bone controllers exceeded" );
		}
		strcpy( controllerNames[numControllers], com_token );
		strlower( controllerNames[numControllers] );
		text = COM_Parse( text );
		if ( !text ) {
			Error( "no bone controller name specified for %s", controllerNames[numControllers] );
		}
		strcpy( controllerTargets[numControllers], com_token );
		strlower( controllerTargets[numControllers] );
		numControllers++;
	}
}

/*
=============
FindMaterial
=============
*/
static materialName_t *FindMaterial( const char *name, materialName_t *skins, int count ) {
	materialName_t *skin;
	int i;
	for ( i = 0; i < count; i++ ) {
		skin = &skins[i];
		if ( !stricmp( name, skin->name ) ) {
			return skin;
		}
	}
	return 0;
}

/*
=============
SkelModel::ParseExportSection
=============
*/
void SkelModel::ParseExportSection( Script &script ) {
	int version;

	script.Skip( "MODEL", true );
	script.Skip( "VERSION", true );
	version = script.GetInteger( false );
	if ( version != 5 && version != 4 ) {
		script.Error( "Expecting version %d or %d but found version %d.\n", 4, 5, version );
	}
	LoadBoneSection( &script );
	if ( model.numbones <= 0 ) {
		Error( "model has no bones" );
	}
	ReadBaseBones( &script );
	LoadVerts( script );
	LoadFaces( script );
}

/*
=============
SkelModel::LoadObjectData
=============
*/
void SkelModel::LoadObjectData( Script &script, int numExtra ) {
	ParseExportSection( script );
	LoadObjects( script );
	LoadMaterials( script, numExtra );
}

/*
=============
SkelModel::LoadSkelGeometryFile
=============
*/
void SkelModel::LoadSkelGeometryFile( const char *filename ) {
	Script script;

	script.LoadFile( filename );
	ParseExportSection( script );
}

/*
=============
SkelModel::LoadModel
=============
*/
void SkelModel::LoadModel( const char *filename, int numExtra ) {
	Script script;

	script.LoadFile( filename );
	LoadObjectData( script, numExtra );
}

/*
=============
SkelModel::SetBoneStabilizers
=============
*/
void SkelModel::SetBoneStabilizers( const char *input ) {
	const char *text;
	numStabilizers = 0;
	for ( text = COM_Parse( input ); text; text = COM_Parse( text ) ) {
		if ( numStabilizers >= 2 ) {
			Error( "maximum bone stabilizers exceeded" );
		}
		strcpy( stabilizerNames[numStabilizers], com_token );
		strlower( stabilizerNames[numStabilizers] );
		text = COM_Parse( text );
		if ( !text ) {
			Error( "no bone stabilizer name specified for %s", stabilizerNames[numStabilizers] );
		}
		strcpy( stabilizerTargets[numStabilizers], com_token );
		strlower( stabilizerTargets[numStabilizers] );
		numStabilizers++;
	}
}

/*
=============
SkelModel::LoadObjects
=============
*/
void SkelModel::LoadObjects( Script &script ) {
	int i;
	char *object;
	int count;
	script.Skip( "NUMOBJECTS", true );
	count = script.GetInteger( false );
	model.objectNames = new char[count][64];
	memset( model.objectNames, 0, count * sizeof( *model.objectNames ) );
	model.numObjects = 0;
	for ( i = 0; i < count; i++ ) {
		script.Skip( "OBJECT", true );
		object = model.objectNames[model.numObjects];
		script.GetInteger( false );
		memset( object, 0, sizeof( model.objectNames[0] ) );
		strncpy( object, script.GetToken( false ), sizeof( model.objectNames[0] ) - 1 );
		model.numObjects++;
	}
}

/*
=============
SkelModel::MaterialUsesRadialNormals
=============
*/
bool SkelModel::MaterialUsesRadialNormals( const char *material ) {
	const char *ptr;
	char *token;

	ptr = shaderLibrary.FindShaderText( material );
	if ( !ptr ) {
		return false;
	}
	token = Com_Parse( &ptr );
	Com_SkipRestOfLine( &ptr );
	if ( strcmp( token, "{" ) ) {
		printf( "*** WARNING: Missing '{' at start of shader [%s] ***\n", material );
		return false;
	}
	for ( ;; ) {
		token = Com_Parse( &ptr );
		if ( !token || !token[0] || !stricmp( token, "}" ) ) break;
		if ( !stricmp( token, "radialNormals" ) ) {
			return true;
		}
		if ( !stricmp( token, "{" ) ) {
			Com_UngetToken();
			Com_SkipBracedSection( &ptr, 0x7fffffff );
		} else {
			Com_SkipRestOfLine( &ptr );
		}
	}
	return false;
}

/*
=============
SkelModel::LoadMaterials
=============
*/
void SkelModel::LoadMaterials( Script &script, int numExtra ) {
	int i;
	materialName_t *material;
	int count, maxMaterials;

	script.Skip( "NUMMATERIALS", true );
	count = script.GetInteger( false );
	maxMaterials = numExtra + count * 2;
	model.materialNames = new materialName_t[maxMaterials];
	memset( model.materialNames, 0, maxMaterials * sizeof( materialName_t ) );
	model.numMaterials = 0;
	for ( i = 0; i < count; i++ ) {
		script.Skip( "MATERIAL", true );
		material = &model.materialNames[model.numMaterials++];
		script.GetInteger( false );
		memset( material->name, 0, sizeof( material->name ) );
		strncpy( material->name, script.GetToken( false ), sizeof( material->name ) - 1 );
		if ( radialNormals ) {
			material->radialNormals = MaterialUsesRadialNormals( material->name );
		}
	}
}

/*
=============
Model::GetNamedMaterial
=============
*/
materialName_t *Model::GetNamedMaterial( char *name ) {
	return ::FindMaterial( name, model.materialNames, model.numMaterials );
}

/*
=============
Model::LoadBoneSection
=============
*/
void Model::LoadBoneSection( Script *const script ) {
	ModelBone *bone;
	int i;
	int j;
	int k;
	int number;
	int parent;
	char szName[64];

	script->Skip( "NUMBONES", true );
	model.numRootBones = script->GetInteger( false );

	model.numbones = model.numRootBones + numControllers + numStabilizers;
	if ( model.numbones > 256 ) {
		script->Error( "Too many bones in model.  Numbones (including bone controllers & bone stabilizers) cannot exceed %d.", 256 );
	}

	model.bones = new ModelBone[model.numbones];
	model.boneRemap = new int[model.numRootBones];

	for ( i = 0; i < model.numRootBones; i++ ) {
		model.boneRemap[i] = i;
	}

	memset( controllerUsed, 0, sizeof( controllerUsed ) );
	memset( stabilizerUsed, 0, sizeof( stabilizerUsed ) );

	bone = model.bones;
	for ( i = 0; i < model.numRootBones; i++, bone++ ) {
		script->Skip( "BONE", true );
		number = script->GetInteger( false );
		if ( number != i ) {
			script->Error( "Bone number %i out of sync", number );
		}

		parent = script->GetInteger( false );
		if ( parent < -1 || parent >= model.numRootBones ) {
			script->Error( "Parent bone out of range for bone %i", number );
		}

		if ( parent != -1 ) {
			parent = model.boneRemap[parent];
		}

		strncpy( szName, script->GetString( false ), 63 );
		strlower( szName );

		for ( j = 0; j < numStabilizers; j++ ) {
			if ( stricmp( szName, stabilizerNames[j] ) ) {
				continue;
			}
			if ( parent == -1 ) {
				script->Error( "Bone stabilizers not allowed on root bone %s", stabilizerNames[j] );
			}

			bone->index = model.boneRemap[i];
			bone->set = false;
			bone->parent = parent;
			memset( bone->name, 0, 64 );
			strncpy( bone->name, stabilizerTargets[j], 63 );
			bone->stabilizer = 1;
			stabilizerUsed[j] = 1;

			for ( k = i; k < model.numRootBones; k++ ) {
				model.boneRemap[k]++;
			}

			parent = bone->index;
			bone++;
			for ( k = j + 1; k < numStabilizers; k++ ) {
				if ( !stricmp( bone->name, stabilizerNames[k] ) ) {
					script->Error( "More than one bone stabilizers on %s", stabilizerNames[k] );
				}
			}
			break;
		}

		bone->index = model.boneRemap[i];
		bone->set = false;
		bone->parent = parent;
		bone->stabilizer = 0;
		memset( bone->name, 0, 64 );
		strncpy( bone->name, szName, 63 );

		for ( j = 0; j < numControllers; j++ ) {
			if ( stricmp( bone->name, controllerNames[j] ) ) {
				continue;
			}

			for ( k = i; k < model.numRootBones; k++ ) {
				model.boneRemap[k]++;
			}
			bone++;

			bone->index = model.boneRemap[i];
			bone->set = false;
			bone->parent = bone->index - 1;
			assert( (bone->parent >= 0) && (bone->parent < model.numbones) );
			bone->stabilizer = 0;
			memset( bone->name, 0, 64 );
			strncpy( bone->name, controllerTargets[j], 63 );
			strlower( bone->name );
			controllerUsed[j] = 1;
			for ( k = j + 1; k < numControllers; k++ ) {
				if ( !stricmp( bone->name, controllerNames[k] ) ) {
					script->Error( "More than one bone controller on %s", controllerNames[k] );
				}
			}
			break;
		}
	}

	for ( j = 0; j < numControllers; j++ ) {
		if ( controllerUsed[j] ) continue;
		Error( "bone '%s' for bone controller '%s' not found", controllerNames[j], controllerTargets[j] );
	}
	for ( j = 0; j < numStabilizers; j++ ) {
		if ( stabilizerUsed[j] ) continue;
		Error( "bone '%s' for bone stabilizer '%s' not found", stabilizerNames[j], stabilizerTargets[j] );
	}
	bone = model.bones;
	for ( i = 0; i < model.numbones; i++, bone++ ) {
		for ( j = i + 1; j < model.numbones; j++ ) {
			if ( !stricmp( bone->name, model.bones[j].name ) ) {
				Error( "duplicate bone '%s'", bone->name );
			}
		}
	}
}

/*
=============
SkelModel::LoadAnimationParts
=============
Map exported animation part indices to the expanded model skeleton.
The optional node restricts the hierarchy retained by this conversion.
Controller and stabilizer bones use the same file-to-model mapping.
Retain each referenced source part before compacting the bone list.
Update parent indices after removing unused entries.
The final remap is shared by frame and base-pose loading.
A missing requested node is a source export error.
The input Script remains positioned at the next animation section.
*/
void SkelModel::LoadAnimationParts( Script &script, const char *nodeName ) {
	ModelBone *bone;
	int i;
	int stabilizer;
	int partNumber;
	const char *partName;
	int bonenum;
	int j;

	// Parts are listed independently of the model skeleton. A part which is
	// absent from the model can still be selected as the motion node.

	script.Skip( "NUMPARTS", true );
	model.numNodeParts = script.GetInteger( false );
	model.numParts = model.numNodeParts + numStabilizers;
	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		model.bones[bonenum].used = false;
	}
	model.animToModel = new short[model.numParts];
	model.nodePart = -1;

	// Resolve the exported part names before adding stabilizer parts.

	for ( i = 0; i < model.numNodeParts; i++ ) {
		script.Skip( "PART", true );
		partNumber = script.GetInteger( false );
		if ( partNumber != i ) {
			script.Error( "Part number out of sync" );
		}
		model.animToModel[partNumber] = -1;
		partName = script.GetString( false );
		bone = model.bones;
		for ( bonenum = 0; bonenum < model.numbones; bonenum++, bone++ ) {
			if ( !stricmp( partName, bone->name ) ) {
				if ( bone->used ) {
					script.Error( "Duplicate part \"%s\" in animation export.", partName );
				}
				bone->used = true;
				model.animToModel[partNumber] = bonenum;
				break;
			}
		}

		// The motion node must remain separate from the model hierarchy.
		if ( nodeName[0] ) {
			if ( !stricmp( partName, nodeName ) ) {
				if ( bonenum != model.numbones ) {
					script.Error( "Node part \"%s\" cannot be in the model.", nodeName );
				}
				if ( model.nodePart != -1 ) {
					script.Error( "Duplicate node part \"%s\" in animation export.", partName );
				}
				model.nodePart = partNumber;
			}
		}
	}

	// Stabilizers use their generated bone names, rather than exported parts.
	// They are appended to the part-to-bone map in stabilizer order.

	for ( stabilizer = 0; i < model.numParts && stabilizer < numStabilizers; i++, stabilizer++ ) {
		model.animToModel[i] = -1;
		partName = stabilizerTargets[stabilizer];
		bone = model.bones;
		for ( bonenum = 0; bonenum < model.numbones; bonenum++, bone++ ) {
			if ( !stricmp( partName, bone->name ) ) {
				if ( bone->used ) {
					script.Error( "Duplicate bone stabilizer part \"%s\" in animation export.", partName );
				}
				bone->used = true;
				model.animToModel[i] = bonenum;
				break;
			}
		}
	}

	if ( nodeName[0] && model.nodePart == -1 ) {
		Error( "Node part \"%s\" not found.", nodeName );
	}

	// Reparent animated bones to their nearest animated ancestor before
	// compacting the table; intermediate unused bones will disappear.

	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		bone = &model.bones[bonenum];
		while ( bone->parent != -1 ) {
			if ( model.bones[bone->parent].used ) break;
			bone->parent = model.bones[bone->parent].parent;
		}
	}

	// Shift the remaining bones and repair both parent and part indices
	// whenever an unused bone is removed.

	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		if ( model.bones[bonenum].used ) continue;
		for ( j = bonenum + 1; j < model.numbones; j++ ) {

			bone = &model.bones[j];
			assert( bone->parent != bonenum );
			if ( bone->parent > bonenum ) {
				bone->parent--;
			}
			model.bones[j - 1] = *bone;
		}

		for ( i = 0; i < model.numParts; i++ ) {
			assert( model.animToModel[i] != bonenum );
			if ( model.animToModel[i] > bonenum ) {
				model.animToModel[i]--;
			}
		}
		model.numbones--;
	}
}


#endif

#ifdef XANIM_CONVERTER
/*
=============
SkelModel::LoadFrames
=============
*/
void SkelModel::LoadFrames( Script &script ) {
	loadbone_t *bone;
	int j;
	int i;
	int number;
	loadbone_t dummyBone;
	int boneIndex;
	int numframes;
	vec3_t offset, scale, matrix[3];
	int framerate;
	int parentIndex;
	int nodeBone;
	loadbone_t *parentBone;
	loadbone_t *originBone;

	nodeBone = -1;
	if ( model.numStabilizers ) {
		originBone = 0;
		for ( i = 0; i < model.numbones; i++ ) {
			if ( stricmp( model.bones[i].name, "tag_origin" ) ) continue;
			nodeBone = i;
			break;
		}
		if ( nodeBone < 0 ) {
			script.Error( "Model does not have a tag_origin bone" );
		}
	}

	script.Skip( "FRAMERATE", true );
	framerate = script.GetInteger( false );
	if ( framerate <= 0 ) {
		script.Error( "Framerate must be positive (using %d).", framerate );
	}
	script.Skip( "NUMFRAMES", true );
	numframes = script.GetInteger( false );
	if ( numframes <= 0 ) {
		script.Error( "numframes must be positive" );
	}
	model.numframes = numframes;
	model.framerate = framerate;
	printf( "framerate: %d\n", model.framerate );

	if ( model.frames ) {
		delete [] model.frames;
	}
	model.frames = new Frame[numframes];
	model.nodeFrames = new loadbone_t[numframes];

	for ( i = 0; i < numframes; i++ ) {
		model.frames[i].bones = new loadbone_t[model.numbones];
		script.Skip( "FRAME", true );
		model.frames[i].number = script.GetInteger( false );
		for ( j = 0; j < model.numNodeParts; j++ ) {
			script.Skip( "PART", true );
			number = script.GetInteger( false );
			if ( number != j ) {
				script.Error( "Bone number out of sync" );
			}
			script.Skip( "OFFSET", true );
			script.GetVector( offset, false );
			script.Skip( "SCALE", true );
			script.GetVector( scale, false );
			script.Skip( "X", true );
			script.GetVector( matrix[0], false );
			script.Skip( "Y", true );
			script.GetVector( matrix[1], false );
			script.Skip( "Z", true );
			script.GetVector( matrix[2], false );

			boneIndex = model.animToModel[j];
			if ( boneIndex >= 0 ) {
				bone = &model.frames[i].bones[boneIndex];
			} else if ( j == model.nodePart ) {
				bone = &model.nodeFrames[i];
			} else {
				bone = &dummyBone;
			}
			VectorCopy( offset, bone->offset );
			VectorCopy( scale, bone->scale );
			VectorCopy( matrix[0], bone->axis[0] );
			VectorCopy( matrix[1], bone->axis[1] );
			VectorCopy( matrix[2], bone->axis[2] );
		}
		for ( ; j < model.numParts; j++ ) {
			boneIndex = model.animToModel[j];
			bone = &model.frames[i].bones[boneIndex];
			parentIndex = model.bones[boneIndex].parent;
			if ( parentIndex < 0 ) {
				script.Error( "Animation does not have a tag_origin bone in it" );
			}
			parentBone = &model.frames[i].bones[parentIndex];
			VectorCopy( parentBone->offset, bone->offset );
			VectorCopy( parentBone->scale, bone->scale );
			originBone = &model.frames[i].bones[nodeBone];
			VectorCopy( originBone->axis[0], bone->axis[0] );
			VectorCopy( originBone->axis[1], bone->axis[1] );
			VectorCopy( originBone->axis[2], bone->axis[2] );
		}
	}
}

/*
=============
SkelModel::AddNoteTrack
=============
*/
void SkelModel::AddNoteTrack( const char *name, int frame ) {
	NotetrackChannel *channel;
	int i, j;

	for ( i = 0; i < model.numChannels; i++ ) {
		channel = &model.channels[i];
		if ( !strcmp( name, channel->name ) ) {
			goto found;
		}
	}
	if ( model.numChannels >= MAX_NOTETRACK_CHANNELS ) {
		Error( "MAX_NOTETRACK_CHANNELS exceeded" );
	}
	channel = &model.channels[model.numChannels];
	if ( strlen( name ) >= NOTETRACK_NAME_LEN ) {
		Error( "NOTETRACK_NAME_LEN exceeded" );
	}
	if ( !strcmp( name, "end" ) ) {
		Error( "'end' is not an allowed user specified notetrack" );
	}
	strcpy( channel->name, name );
	channel->numFrames = 0;
	model.numChannels++;

found:
	for ( i = 0; i < channel->numFrames; i++ ) {
		if ( frame <= channel->frames[i] ) {
			if ( frame == channel->frames[i] ) {
				return;
			}
			break;
		}
	}
	if ( channel->numFrames >= MAX_NOTETRACK_FRAMES ) {
		Error( "MAX_NOTETRACK_FRAMES exceeded" );
	}
	for ( j = channel->numFrames - 1; j >= i; j-- ) {
		channel->frames[j + 1] = channel->frames[j];
	}
	channel->frames[i] = frame;
	channel->numFrames++;
}

/*
=============
SkelModel::GetFrameIndex
=============
*/
int SkelModel::GetFrameIndex( int number ) {
	int i;

	for ( i = model.numframes - 1; i >= 0; i-- ) {
		if ( model.frames[i].number == number ) {
			break;
		}
	}
	return i;
}

/*
=============
TrimWhitespace
=============
*/
static char *TrimWhitespace( char *s ) {
	int len;

	while ( iswspace( *s ) ) {
		s++;
	}
	for ( len = strlen( s ) - 1; len >= 0; len-- ) {
		if ( !iswspace( s[len] ) ) break;
		s[len] = 0;
	}
	return s;
}

/*
=============
SkelModel::LoadNoteTracks
=============
*/
void SkelModel::LoadNoteTracks( Script &script ) {
	int k;
	int j;
	int i;
	int number;
	int numTracks;
	int numKeys;
	char *lowerName;
	int len;
	int frameNumber;
	int frame;
	const char *name;
	int n;

	model.numChannels = 0;
	script.Skip( "NOTETRACKS", true );
	for ( i = 0; i < model.numNodeParts; i++ ) {
		script.Skip( "PART", true );
		number = script.GetInteger( false );
		if ( number != i ) {
			script.Error( "Bone number out of sync" );
		}
		script.Skip( "NUMTRACKS", true );
		numTracks = script.GetInteger( false );
		for ( j = 0; j < numTracks; j++ ) {
			script.Skip( "NOTETRACK", true );
			number = script.GetInteger( false );
			if ( number != j ) {
				script.Error( "Notetrack number out of sync" );
			}
			script.Skip( "NUMKEYS", true );
			numKeys = script.GetInteger( false );
			for ( k = 0; k < numKeys; k++ ) {
				script.Skip( "FRAME", true );
				frameNumber = script.GetInteger( false );
				name = script.GetToken( false );
				frame = GetFrameIndex( frameNumber );
				if ( frame < 0 ) continue;
				len = strlen( name ) + 1;
				lowerName = (char *)_alloca( len );
				for ( n = len - 1; n >= 0; n-- ) {
					lowerName[n] = tolower( name[n] );
				}
				lowerName = TrimWhitespace( lowerName );
				AddNoteTrack( lowerName, frame );
			}
		}
	}
}
#endif

#ifndef XANIM_CONVERTER
// Keep an export-index remap while merging equivalent input vertices.
/*
VertsEqual

Compare positions, blend-bone identities and their weights.
The tolerant path allows position differences within the export tolerance.
Blend counts and bone assignments must still correspond.
*/
bool VertsEqual
	(
	loadvertx_t *a,
	loadvertx_t *b,
	int tolerant
	)
	{
	const blendvert_t *blendA, *blendB;
	int j, i;
	float tolerance;

	if ( a->numbones != b->numbones ) {
		return false;
	}
	if ( !tolerant ) {
		tolerance = 0;
	} else {
		tolerance = a->maxEdgeLength > b->maxEdgeLength ? a->maxEdgeLength : b->maxEdgeLength;
	}
	if ( VectorDistanceSquared( a->pos, b->pos ) > tolerance * tolerance * 0.01f * 0.01f ) {
		return false;
	}
	blendA = a->blend;
	for ( i = 0; i < a->numbones; i++, blendA++ ) {
		blendB = b->blend;
		for ( j = 0; j < b->numbones; j++, blendB++ ) {
			if ( blendA->bone != blendB->bone ) continue;
			if ( fabs( blendA->weight - blendB->weight ) < 0.01f ) break;
			return false;
		}
		if ( j == b->numbones ) {
			return false;
		}
	}
	return true;
}

/*
Model::FindVert

Search the vertices already read for an equivalent exported vertex.
Use the same equivalence rule that drives the input remap.
Return the matching index when a prior vertex can be reused.
A failed search leaves this vertex as a new model vertex.
*/
int Model::FindVert
	(
	loadvertx_t *vert
	) const
	{
	loadvertx_t *other;
	int i;

	for ( i = 0; i < model.numverts; i++ ) {
		other = &model.verts[i];

		if ( ::VertsEqual( vert, other, vert->merged || other->merged ) ) {
			return i;
		}
	}
	return model.numverts;
}

/*
SkelModel::LoadVerts

Read the numbered VERT entries following NUMVERTS.
Each exported vertex supplies a position and a list of weighted bones.
Resolve file bone numbers through the expanded model skeleton.
Merge equivalent vertices while preserving the export-to-model remap.
Faces can then refer to their original vertex numbers.
*/
void SkelModel::LoadVerts
	(
	Script &script
	)
	{
	blendvert_t *blend;
	loadvertx_t *vert;
	int j, i, index;

	script.Skip( "NUMVERTS", true );
	model.numSourceVerts = script.GetInteger( false );
	model.verts = new loadvertx_t[model.numSourceVerts];
	model.vertsSaved = new loadvertx_t[model.numSourceVerts];
	memset( model.verts, 0, model.numSourceVerts * sizeof( loadvertx_t ) );
	model.vertRemap = new int[model.numSourceVerts];
	for ( i = 0; i < model.numSourceVerts; i++ ) {
		vert = &model.verts[i];
		script.Skip( "VERT", true );
		index = script.GetInteger( false );
		if ( index != i ) {
			script.Error( "Vertex number out of sync" );
		}
		script.Skip( "OFFSET", true );
		script.GetVector( vert->pos, false );
		script.Skip( "BONES", true );
		vert->numbones = script.GetInteger( false );
		if ( vert->numbones >= MAX_VERT_BLENDS ) {
			Error( "Exceeded %d bones on vertex %d\n", MAX_VERT_BLENDS, i );
		}
		for ( blend = vert->blend, j = 0; j < vert->numbones; j++, blend++ ) {
			script.Skip( "BONE", true );
			blend->bone = script.GetInteger( false );
			if ( blend->bone < 0 || blend->bone >= model.numRootBones ) {
				script.Error( "Vertex %d has bad bone\n", j, blend->bone );
			}
			blend->bone = model.boneRemap[blend->bone];
			blend->weight = script.GetFloat( false );
			if ( blend->weight < 0 ) {
				script.Error( "Vertex %d has negative weight %.2f\n", j, blend->weight );
			}
		}
		ReduceBonesPerVertex( vert, i );
	}
}

/*
SkelModel::LoadFaces

Read numbered triangle records after NUMFACES.
Each corner retains its texture coordinates and exported normal.
Map the source vertex index to the vertex retained by LoadVerts.
The object and material indices group triangles into output surfaces.
The corner links are built later when the surfaces are assembled.
The surface builder later filters triangles with repeated vertex indices.
*/
void SkelModel::LoadFaces
	(
	Script &script
	)
	{
	FaceCorner *corner;
	ModelFace *face;
	int j, i;

	script.Skip( "NUMFACES", true );
	model.numfaces = script.GetInteger( false );
	model.faces = new ModelFace[model.numfaces];
	face = model.faces;
	for ( i = 0; i < model.numfaces; i++, face++ ) {
		script.Skip( "TRI", true );
		face->object = script.GetInteger( false );
		face->material = script.GetInteger( false );
		if ( face->material == -1 ) {
			script.Error( "Face with no material - xmodel needs to be re-exported" );
		}
		face->unused = script.GetInteger( false );
		face->bUsed = false;
		script.GetInteger( false );
		corner = face->corners;
		for ( j = 0; j < 3; j++, corner++ ) {
			int index;

			script.Skip( "VERT", true );
			index = script.GetInteger( false );
			if ( index < 0 || index >= model.numSourceVerts ) {
				script.Error( "Vertex index out of range" );
			}
			corner->index = index;
			corner->st[0] = script.GetFloat( false );
			corner->st[1] = script.GetFloat( false );
			corner->normal[0] = script.GetFloat( false );
			corner->normal[1] = script.GetFloat( false );
			corner->normal[2] = script.GetFloat( false );
			VectorNormalize( corner->normal );
		}
	}
}

/*
=============
Model::ReadBaseBones
=============
*/
void Model::ReadBaseBones( Script *const script ) {
	BonePose *otherPose;
	BonePose *pose;
	BonePose *tagPose;

	int i;
	int index;

	tagPose = 0;

	model.numframes = 1;
	model.poses = new BonePoses[1];
	model.poses[0].pose = new BonePose[model.numbones];

	for ( i = 0; i < model.numbones; i++ ) {
		pose = &model.poses[0].pose[i];
		pose->set = 0;
	}

	for ( i = 0; i < model.numRootBones; i++ ) {
		script->Skip( "BONE", true );

		index = script->GetInteger( false );
		if ( index != i ) {
			script->Error( "Bone number out of sync" );
		}

		pose = &model.poses[0].pose[model.boneRemap[i]];
		pose->set = 1;

		script->Skip( "OFFSET", true );
		script->GetVector( pose->origin, false );

		script->Skip( "SCALE", true );
		script->GetVector( pose->scale, false );

		script->Skip( "X", true );
		script->GetVector( pose->axis[0], false );

		script->Skip( "Y", true );
		script->GetVector( pose->axis[1], false );

		script->Skip( "Z", true );
		script->GetVector( pose->axis[2], false );
	}

	if ( numStabilizers ) {

		for ( i = 0; i < model.numbones; i++ ) {
			if ( stricmp( model.bones[i].name, "tag_origin" ) ) {
				continue;
			}
			tagPose = &model.poses[0].pose[i];
			break;
		}

		if ( !tagPose ) {
			script->Error( "Model does not have a tag_origin bone" );
		}
	}

	for ( i = 0; i < model.numbones; i++ ) {
		pose = &model.poses[0].pose[i];

		if ( pose->set ) {
			continue;
		}

		assert( i != model.numbones - 1 );
		if ( model.bones[i].stabilizer ) {
			otherPose = &model.poses[0].pose[model.bones[i].parent];

			VectorCopy( otherPose->origin, pose->origin );
			VectorCopy( otherPose->scale, pose->scale );
			VectorCopy( tagPose->axis[0], pose->axis[0] );
			VectorCopy( tagPose->axis[1], pose->axis[1] );
			VectorCopy( tagPose->axis[2], pose->axis[2] );
		} else {
			otherPose = &model.poses[0].pose[i + 1];

			VectorCopy( otherPose->origin, pose->origin );
			VectorCopy( otherPose->scale, pose->scale );
			VectorCopy( otherPose->axis[0], pose->axis[0] );
			VectorCopy( otherPose->axis[1], pose->axis[1] );
			VectorCopy( otherPose->axis[2], pose->axis[2] );
		}
	}
}

/*
SkelModel::VertexCompare
*/
bool SkelModel::VertexCompare
	(
	int a,
	int b
	)

	{
	float tolerance;
	int i;

	if ( a == b ) {
		return true;
	}
	if ( model.verts[a].numbones != model.verts[b].numbones ) {
		return false;
	}
	tolerance = model.verts[a].numbones == 1 ? 0.2f * 0.2f : 0.001f * 0.001f;
	if ( VectorDistanceSquared( model.verts[a].pos, model.verts[b].pos ) > tolerance ) {
		return false;
	}
	for ( i = 0; i < model.verts[a].numbones; i++ ) {
		if ( model.verts[a].blend[i].bone != model.verts[b].blend[i].bone ) {
			return false;
		}
		if ( model.verts[a].blend[i].weight != model.verts[b].blend[i].weight ) {
			return false;
		}
	}
	return true;
}

/*
SkelModel::AppendSurfaceVert

Reuse a surface vertex when position, texture coordinates and normal agree.
Material radial-normal policy can replace the exported normal.
Different rigid-bone assignments force the shared vertex to remain blended.
*/
int SkelModel::AppendSurfaceVert
	(
	surface_t *const surf,
	FaceCorner *corner,
	unsigned int object,
	int material
	)

	{
	int i;
	SurfVert *surfVert;
	int vertIndex;
	float texCoords[2];
	vec3_t normal;
	int bone;
	float texCoordDiff[2];
	float tolerance, normalTolerance;

	vertIndex = corner->index;
	texCoords[0] = corner->st[0];
	texCoords[1] = corner->st[1];
	if ( model.materialNames[material].radialNormals ) {
		normal[0] = model.verts[vertIndex].pos[0] - radialOrigin[0];
		normal[1] = model.verts[vertIndex].pos[1] - radialOrigin[1];
		normal[2] = model.verts[vertIndex].pos[2] - radialOrigin[2];
		VectorNormalize( normal );
	} else {
		normal[0] = corner->normal[0];
		normal[1] = corner->normal[1];
		normal[2] = corner->normal[2];
	}
	bone = !viewmodel && model.verts[vertIndex].numbones == 1 ? model.verts[vertIndex].blend[0].bone : -1;
	if ( model.verts[vertIndex].merged ) {
		tolerance = 0.01f;
		normalTolerance = 0.01f;
	} else {
		tolerance = 1e-5f;
		normalTolerance = 1.0f / 255;
	}
	surfVert = surf->verts;
	for ( i = 0; i < surf->numverts; i++, surfVert++ ) {
		texCoordDiff[0] = fabs( surfVert->st[0] - texCoords[0] );
		texCoordDiff[1] = fabs( surfVert->st[1] - texCoords[1] );

		if ( VertexCompare( surfVert->index, vertIndex ) && texCoordDiff[0] <= tolerance && texCoordDiff[1] <= tolerance
			&& VectorDistance( surfVert->normal, normal ) <= normalTolerance ) {
			if ( surfVert->bone != bone ) {
				surfVert->bone = -1;
			}
			return i;
		}
	}
	if ( surf->numverts >= MAX_SURF_VERTS ) {
		Error( "Exceeded %d vertices on surface '%s' of object '%s'\n", MAX_SURF_VERTS, model.materialNames[material].name, model.objectNames[object] );
	}
	surfVert->index = vertIndex;
	surfVert->bone = bone;
	surfVert->st[0] = texCoords[0];
	surfVert->st[1] = texCoords[1];
	surfVert->normal[0] = normal[0];
	surfVert->normal[1] = normal[1];
	surfVert->normal[2] = normal[2];
	surf->numverts++;
	return i;
}

/*
SkelModel::AppendSurfaceTriangle

Append the three corners through the surface-vertex deduplicator.
Keep only triangles whose three resulting vertex indices are distinct.
Restore the original vertex count when the triangle degenerates.
*/
int SkelModel::AppendSurfaceTriangle
	(
	surface_t *surf,
	FaceCorner *corners,
	int object,
	int material
	)

	{
	int i;
	int numtris;
	int *index;
	int numverts;

	if ( surf->numtris >= MAX_SURF_TRIS ) {
		Error( "Exceeded %d triangles on surface '%s' of object '%s'\n", MAX_SURF_TRIS, model.materialNames[material].name, model.objectNames[object] );
	}
	numtris = surf->numtris;
	numverts = surf->numverts;
	index = surf->tris[numtris].index;
	for ( i = 0; i < 3; i++ ) {
		index[i] = AppendSurfaceVert( surf, &corners[i], object, material );
	}
	if ( index[0] != index[1] && index[0] != index[2] && index[1] != index[2] ) {
		surf->numtris++;
		return numtris;
	}
	surf->numverts = numverts;
	return -1;
}

/*
=============
ResolveVertexBones
=============
*/
static int ResolveVertexBones( surface_t *surf ) {
	int i;
	int changed;

	changed = 0;

	for ( i = 0; i < surf->numtris; i++ ) {
		if ( surf->verts[surf->tris[i].index[0]].bone == surf->verts[surf->tris[i].index[1]].bone && surf->verts[surf->tris[i].index[0]].bone == surf->verts[surf->tris[i].index[2]].bone ) continue;
		surf->verts[surf->tris[i].index[0]].bone = -1;
		surf->verts[surf->tris[i].index[1]].bone = -1;
		surf->verts[surf->tris[i].index[2]].bone = -1;
		changed = 1;
	}
	return changed;
}

/*
Object and material indices select the contributing source triangles.
Append their corners through the surface vertex deduplicator.
The temporary surface owns its vertex and triangle arrays.
Resolve repeated bone assignments before committing that surface.
Discard groups that contain no usable triangles.
*/
void SkelModel::CreateSurface
	(
	int object,
	int mtlID
	)

	{
	int j, i;
	surface_t *surf;
	ModelFace *face;
	int bone;
	int firstSurface;
	surface_t tempSurf;


	assert( mtlID >= 0 );

	firstSurface = model.numsurfaces;

	memset( &tempSurf, 0, sizeof( tempSurf ) );

	for ( face = model.faces, i = 0; i < model.numfaces; i++, face++ ) {

		if ( face->object != object || face->material != mtlID ) {
			continue;
		}

		assert( !face->bUsed );
		face->bUsed = true;
		face->loadIndex = AppendSurfaceTriangle(
			&tempSurf,
			face->corners,
			face->object,
			mtlID );
		if ( face->loadIndex >= 0 ) {
			model.unusedFaces--;
		} else {
			printf(
				"*** WARNING: culling face from material '%s'\n",
				model.materialNames[mtlID].name );
			face->material = -1;
		}
	}

	while ( ResolveVertexBones( &tempSurf ) ) {
	}

	for ( face = model.faces, i = 0; i < model.numfaces; i++, face++ ) {

		if ( face->object != object || face->material != mtlID ) {
			continue;
		}

		assert( face->loadIndex >= 0 );
		bone = tempSurf.verts[tempSurf.tris[face->loadIndex].index[0]].bone;
		for ( j = firstSurface; j < model.numsurfaces; j++ ) {
			surf = &model.surfaces[j];
			if ( surf->boneIndex == bone ) {
				break;
			}
		}
		if ( j == model.numsurfaces ) {
			surf = GetNewSurface( object, mtlID, bone );
		}
		AppendSurfaceTriangle( surf, face->corners, face->object, mtlID );
	}
}

/*
SkelModel::GetNewSurface
*/
surface_t *SkelModel::GetNewSurface
	(
	int object,
	int material,
	int bone
	)

	{
	surface_t *surf;
	surface_t *surfaces;
	int i;

	if ( model.numsurfaces == model.maxsurfaces ) {
		model.maxsurfaces *= 2;
		surfaces = new surface_t[model.maxsurfaces];
		memset( surfaces, 0, model.maxsurfaces * sizeof( surface_t ) );
		for ( i = 0; i < model.numsurfaces; i++ ) {
			surfaces[i] = model.surfaces[i];
		}
		delete[] model.surfaces;
		model.surfaces = surfaces;
	}
	surf = &model.surfaces[model.numsurfaces];
	surf->object = object;
	surf->material = material;
	surf->numtris = 0;
	surf->numverts = 0;
	surf->boneIndex = bone;
	model.numsurfaces++;
	return surf;
}

void SkelModel::CreateSurfaces
	(
	)

	{
	int i, j;

	model.maxsurfaces = model.numObjects * model.numMaterials;
	model.surfaces = new surface_t[model.maxsurfaces];
	memset( model.surfaces, 0, model.maxsurfaces * sizeof( surface_t ) );
	model.unusedFaces = model.numfaces;
	model.numsurfaces = 0;
	for ( j = 0; j < model.numMaterials; j++ ) {
		for ( i = 0; i < model.numObjects; i++ ) {
			CreateSurface( i, j );
		}
	}
	delete[] model.faces;
	model.faces = NULL;
}

/*
SkelModel::ComputeBoundingBox
*/
void SkelModel::ComputeBoundingBox ( xmodelInfo_t &info ) const {
	BonePose *pose;
	vec3_t vtx, temp, mins, maxs, size;
	float matInv[3][3];
	float length, radius;
	int i;

	if ( model.numverts == 3 ) {
		VectorClear( info.mins );
		VectorClear( info.maxs );
		return;
	}
	if ( model.numbones > 1 ) {
		printf( "*** WARNING: Calculating bounds on a model with more than 1 bone\n" );
	}
	pose = model.poses[0].pose;
	mins[0] = FLT_MAX;
	mins[1] = FLT_MAX;
	mins[2] = FLT_MAX;
	maxs[0] = -FLT_MAX;
	maxs[1] = -FLT_MAX;
	maxs[2] = -FLT_MAX;
	radius = 0;
	for ( i = 0; i < model.numverts; i++ ) {
		VectorSubtract( model.verts[i].pos, pose->origin, temp );
		MatrixTranspose( pose->axis, matInv );
		MatrixTransformVector( temp, matInv, vtx );
		AddPointToBounds( vtx, mins, maxs );
		length = sqrt( vtx[0] * vtx[0] + vtx[1] * vtx[1] + vtx[2] * vtx[2] );
		if ( radius < length ) {
			radius = length;
		}
	}
	size[0] = maxs[0] - mins[0];
	size[1] = maxs[1] - mins[1];
	size[2] = maxs[2] - mins[2];
	if ( radius > 32 ) {
		if ( sqrt( size[0] * size[0] + size[1] * size[1] + size[2] * size[2] ) * 1.732f < radius ) {
			printf( "actual mins/maxs (%g %g %g) (%g %g %g)\n", mins[0], mins[1], mins[2], maxs[0], maxs[1], maxs[2] );
			printf( "actual size (%g %g %g) radius %g\n", size[0], size[1], size[2], sqrt( size[0] * size[0] + size[1] * size[1] + size[2] * size[2] ) * 0.5f );
			printf( "rotation invariant radius %g\n", radius );
			Error( "auto-generated rotation invariant bounds far exceed physical bounds; model should be exported closer to the root bone\n" );
		}
	}
	VectorSet( info.mins, -radius, -radius, -radius );
	VectorSet( info.maxs, radius, radius, radius );
}

/*
ShaderInfo::ShaderInfo
*/
ShaderInfo::ShaderInfo ( ) {
	cv = 0;
	types = 0;
	text = 0;
	files = 0;
	loaded = false;
}

/*
ShaderLibrary::~ShaderLibrary
*/
ShaderInfo::~ShaderInfo ( ) {
	ShaderType *type;
	ShaderFile *file;

	while ( types ) {
		type = types;
		types = types->next;
		free( type->text );
		free( type );
	}
	while ( files ) {
		file = files;
		files = files->next;
		free( file );
	}
	free( text );
	loaded = false;
}

// Lookup strings remain valid while this cache owns the loaded shader files.
/*
ShaderInfo::FindShaderTypeText
*/
const char *ShaderInfo::FindShaderTypeText
	(
	const char *name
	)

	{
	ShaderType *type;
	char fileName[1024];

	for ( type = types; type; type = type->next ) {
		if ( !stricmp( type->name, name ) ) {
			cv->AddSourceFile( type->path );
			return type->text;
		}
	}
	type = (ShaderType *)malloc( sizeof( ShaderType ) );
	strcpy( type->name, name );
	type->next = types;
	types = type;
	sprintf( fileName, "%sshadertypes/model/%s.stype", dir, name );
	if ( TryLoadFile( fileName, (void **)&type->text ) < 0 ) {
		sprintf( fileName, "%sshadertypes/model/%s.stype", gameDir, name );
		if ( TryLoadFile( fileName, (void **)&type->text ) < 0 ) {
			printf( "*** WARNING: Can't find shader type [%s] ***\n", fileName );
			type->text = 0;
		}
	}
	strcpy( type->path, fileName );
	cv->AddSourceFile( fileName );
	return type->text;
}

/*
ShaderInfo::ReadShaderFileData

Enumerate shader scripts from the selected game directory.
Keep the loaded text alive while the shader lookup entries refer into it.
Cache the file list so later material lookups reuse the same text.
*/
char *ShaderInfo::ReadShaderFileData
	(
	)

	{
	WIN32_FIND_DATA wfd;
	HANDLE handle;
	int length, size, offset;
	FILE *f;
	char szSearchSpec[1024];
	ShaderFile *file;

	if ( loaded ) {
		return text;
	}
	loaded = true;
	sprintf( szSearchSpec, "%s%s", gameDir, "scripts/*.shader" );
	handle = FindFirstFile( szSearchSpec, &wfd );
	if ( handle == INVALID_HANDLE_VALUE ) {
		return 0;
	}
	size = 1;
	do {
		file = (ShaderFile *)malloc( sizeof( ShaderFile ) );
		if ( !file ) {
			return 0;
		}
		sprintf( file->path, "%s%s%s", gameDir, "scripts/", wfd.cFileName );
		file->next = files;
		files = file;
		f = fopen( file->path, "rb" );
		fseek( f, 0, SEEK_END );
		size += ftell( f ) + 1;
		fclose( f );
	} while ( FindNextFile( handle, &wfd ) );
	FindClose( handle );
	text = (char *)malloc( size );
	if ( !text ) {
		return 0;
	}
	offset = 0;
	for ( file = files; file; file = file->next ) {
		f = fopen( file->path, "rb" );
		fseek( f, 0, SEEK_END );
		length = ftell( f );
		fseek( f, 0, SEEK_SET );
		if ( offset + length >= size ) {
			Error( "error filling shader text buffer... shader file sizes grew between finding and reading files\n" );
		}
		fread( text + offset, 1, length, f );
		fclose( f );
		offset += length;
		text[offset++] = '\n';
	}
	text[offset++] = 0;
	return text;
}

/*
ShaderInfo::FindShaderText
*/
const char *ShaderInfo::FindShaderText
	(
	const char *name
	)

	{
	char shaderName[1024];
	char *at;
	const char *buf;
	char *token;
	int num;
	ShaderFile *file;


	at = strrchr( name, '@' );
	if ( at ) {

		num = at - name;
		assert( num > 0 );
		strncpy( shaderName, name, num );
		shaderName[num] = 0;
		return FindShaderTypeText( shaderName );
	}
	buf = ReadShaderFileData();
	if ( !buf ) {
		printf( "*** WARNING: Can't find any shader files ***\n" );
		return 0;
	}
	Com_BeginParseSession( "shaders" );
	for ( ;; ) {
		token = Com_Parse( &buf );
		if ( !*token ) break;
		Com_SkipRestOfLine( &buf );
		if ( !stricmp( token, name ) ) {
			for ( file = files; file; file = file->next ) {
				cv->AddSourceFile( file->path );
			}
			Com_EndParseSession();
			return buf;
		}
		Com_SkipBracedSection( &buf, 0x7fffffff );
	}
	Com_EndParseSession();
	return 0;
}

/*
SkelModel::GetSurfaceParms

Look up the material shader and parse its surface parameters.
Content and surface flags are accumulated independently.
Unknown parameters leave the existing flags unchanged.
The material name selects the cached shader definition.
Accumulate content flags and surface flags independently.
Recognized surface parameters can contribute to either output mask.
Unrecognized parameters leave the current masks unchanged.
*/
void SkelModel::GetSurfaceParms
	(
	const char *shaderName,
	int *contentFlags,
	int *surfaceFlags
	)

	{
	char *parm, *token;
	const char *ptr;
	int i;

	*contentFlags = CONTENTS_SOLID;
	*surfaceFlags = 0;
	ptr = shaderLibrary.FindShaderText( shaderName );
	if ( !ptr ) {
		printf( "*** WARNING: Can't find shader [%s] for collision surface ***\n", shaderName );
		return;
	}
	token = Com_Parse( &ptr );
	if ( strcmp( token, "{" ) ) {
		printf( "*** WARNING: Missing '{' at start of shader [%s] ***\n", shaderName );
		return;
	}
	Com_SkipRestOfLine( &ptr );
	for ( ;; ) {
		token = Com_Parse( &ptr );
		if ( !token || !*token || !strcmp( token, "}" ) ) break;
		if ( !strcmp( token, "{" ) ) {
			Com_UngetToken();
			Com_SkipBracedSection( &ptr, 0x7fffffff );
		} else if ( !strcmp( token, "surfaceparm" ) ) {
			parm = Com_Parse( &ptr );
			for ( i = 0; infoParms[i].name; i++ ) {
				if ( !stricmp( parm, infoParms[i].name ) ) {
					if ( infoParms[i].clearSolid ) {
						*contentFlags &= ~CONTENTS_SOLID;
					}
					*contentFlags |= infoParms[i].contents;
					*surfaceFlags |= infoParms[i].surfaceFlags;
					break;
				}
			}
			if ( !infoParms[i].name ) {
				if ( stricmp( parm, "castshadow" ) && stricmp( parm, "noshadow" ) && strnicmp( parm, "qer", 3 ) ) {
					printf( "*** WARNING: Could not find [%s] surfaceparm\n", parm );
				}
			}
		}
	}
}

/*
SkelModel::BuildCollisionModel

Create collision surfaces from the selected model geometry.
Resolve the material flags before deciding which triangles contribute.
Transform the retained geometry into the collision model space.
The caller receives the collision data through its explicit output model.
Only surfaces selected for collision contribute to the output.
Material parameters determine their content and surface masks.
Transform each triangle into the model bone coordinate space.
Store a plane and texture-space axes for each retained triangle.
The output model owns its allocated surface and triangle arrays.
*/
void SkelModel::GenerateCollisionModel
	(
	loadcollmodel_t &collmodel
	)

	{
	CollSurface *csurf;
	CollTri *tri;
	surface_t *surf;
	loadvertx_t *vert;
	blendvert_t *blend;
	BonePose *pose;
	vec3_t svec, tvec, vtx0, vtx1, vtx2, temp;
	float length, length2, dot, scale;
	float matInv[3][3];
	int i;
	int j;

	collmodel.numCollSurfs = model.numsurfaces;
	assert( collmodel.numCollSurfs <= MAX_COLL_SURFS );

	for ( i = 0; i < collmodel.numCollSurfs; i++ ) {
		csurf = &collmodel.collSurfs[i];
		surf = &model.surfaces[i];

		if ( surf->boneIndex == -1 ) {
			Error( "Collision surf must be rigid bound\n" );
			return;
		}

		csurf->boneIdx = surf->boneIndex;
		VectorSet( csurf->mins,
			FLT_MAX,
			FLT_MAX,
			FLT_MAX );
		VectorSet( csurf->maxs,
			-FLT_MAX,
			-FLT_MAX,
			-FLT_MAX );

		assert( surf->numverts );

		for ( j = 0; j < surf->numverts; j++ ) {
			vert = &model.verts[surf->verts[j].index];
			blend = vert->blend;
			pose = &model.poses[0].pose[blend->bone];
			VectorSubtract( vert->pos, pose->origin, temp );
			MatrixTranspose( pose->axis, matInv );
			MatrixTransformVector( temp, matInv, vtx0 );
			if ( csurf->mins[0] > vtx0[0] )
				csurf->mins[0] = vtx0[0];
			if ( csurf->maxs[0] < vtx0[0] )
				csurf->maxs[0] = vtx0[0];
			if ( csurf->mins[1] > vtx0[1] )
				csurf->mins[1] = vtx0[1];
			if ( csurf->maxs[1] < vtx0[1] )
				csurf->maxs[1] = vtx0[1];
			if ( csurf->mins[2] > vtx0[2] )
				csurf->mins[2] = vtx0[2];
			if ( csurf->maxs[2] < vtx0[2] )
				csurf->maxs[2] = vtx0[2];
		}

		GetSurfaceParms( model.materialNames[surf->material].name, &csurf->contents, &csurf->surfaceFlags );

		csurf->numCollTris = surf->numtris;
		assert( csurf->numCollTris <= MAX_TRIANGLES );

		assert( csurf->numCollTris );
		for ( j = 0; j < csurf->numCollTris; j++ ) {
			tri = &csurf->tris[j];
			vert = &model.verts[surf->verts[surf->tris[j].index[0]].index];
			blend = vert->blend;
			pose = &model.poses[0].pose[blend->bone];
			VectorSubtract( vert->pos, pose->origin, temp );
			MatrixTranspose( pose->axis, matInv );
			MatrixTransformVector( temp, matInv, vtx0 );
			vert = &model.verts[surf->verts[surf->tris[j].index[1]].index];
			blend = vert->blend;
			pose = &model.poses[0].pose[blend->bone];
			VectorSubtract( vert->pos, pose->origin, temp );
			MatrixTranspose( pose->axis, matInv );
			MatrixTransformVector( temp, matInv, vtx1 );
			vert = &model.verts[surf->verts[surf->tris[j].index[2]].index];
			blend = vert->blend;
			pose = &model.poses[0].pose[blend->bone];
			VectorSubtract( vert->pos, pose->origin, temp );
			MatrixTranspose( pose->axis, matInv );
			MatrixTransformVector( temp, matInv, vtx2 );

			PlaneFromPoints( tri->plane, vtx0, vtx1, vtx2 );
			VectorSubtract( vtx1, vtx0, svec );
			VectorSubtract( vtx2, vtx0, tvec );
			length = VectorNormalize( svec );
			length2 = VectorNormalize( tvec );
			dot = DotProduct( svec, tvec );
			scale = 1 / ( 1 - dot * dot );
			VectorScale( svec, scale, svec );
			VectorScale( tvec, scale, tvec );

			VectorMA( svec, -dot, tvec, tri->svec );
			VectorScale( tri->svec, 1 / length, tri->svec );
			tri->svec[3] = DotProduct( vtx0, tri->svec );
			VectorMA( tvec, -dot, svec, tri->tvec );
			VectorScale( tri->tvec, 1 / length2, tri->tvec );
			tri->tvec[3] = DotProduct( vtx0, tri->tvec );
		}
	}
}

/*
=============
SkelModel::ComputeVertexNormals
=============
The source normals are expressed in the exported model coordinate space.
Each surface corner refers to a vertex and its leading blend bone.
Use that bone from the first base pose as the normal coordinate frame.
Transpose the pose rotation before transforming the direction.
Translation does not contribute to this direction transform.
Write the transformed normal back to the surface corner.
The vertex weights and source positions remain available to the writer.
A contributing vertex must have at least one bone assignment.
*/
void SkelModel::ComputeVertexNormals() {
	surface_t *surf;

	vec3_t p;
	float matInv[3][3];
	int i;
	int j;

	surf = model.surfaces;

	for ( i = 0; i < model.numsurfaces; i++, surf++ ) {
		for ( j = 0; j < surf->numverts; j++ ) {

			assert( model.verts[surf->verts[j].index].numbones > 0 );
			MatrixTranspose( model.poses[0].pose[model.verts[surf->verts[j].index].blend[0].bone].axis, matInv );
			MatrixTransformVector( surf->verts[j].normal, matInv, p );
			surf->verts[j].normal[0] = p[0];
			surf->verts[j].normal[1] = p[1];
			surf->verts[j].normal[2] = p[2];
		}
	}
}

/*
=============
SkelModel::CheckSurfaceTiling
=============
Classify texture coordinates at duplicated position boundaries.
First identify seams whose coordinates differ by one texture period.
Use a small seam tolerance for those boundary comparisons.
Then measure the remaining coordinate range over the surface.
A near-unit span with boundary vertices can indicate accidental tiling.
Keep the seam state on the surface for subsequent export decisions.
Report the source material so the artist can locate the affected surface.
This classification changes no vertex position or texture coordinate.
*/
void SkelModel::CheckSurfaceTiling()
{
	surface_t *surf;
	int s, i, j;
	float delta[2], mins[2], maxs[2];
	int imins[2], imaxs[2];
	float snapEpsilon = 0.003f;
	float warningEpsilon = 0.05f;
	float seamEpsilon = 0.01f;

	surf = model.surfaces;
	for (s = 0; s < model.numsurfaces; s++, surf++)
	{
		surf->state = 0;
		if (!surf->numverts)
			continue;
		for (i = 1; i < surf->numverts; i++)
		{
			for (j = 0; j < i; j++)
			{
				if (!VectorCompareEpsilon(model.verts[surf->verts[i].index].pos,
										  model.verts[surf->verts[j].index].pos))
					continue;
				delta[0] = (float)fabs(surf->verts[i].st[0] - surf->verts[j].st[0]);
				delta[1] = (float)fabs(surf->verts[i].st[1] - surf->verts[j].st[1]);
				if (delta[0] < seamEpsilon)
				{
					if (fabs(delta[1] - 1.0f) < seamEpsilon)
					{
						surf->state = 2;
						goto nextSurface;
					}
				}
				else if (delta[1] < seamEpsilon)
				{
					if (fabs(delta[0] - 1.0f) < seamEpsilon)
					{
						surf->state = 2;
						goto nextSurface;
					}
				}
			}
		}
		mins[0] = maxs[0] = surf->verts[0].st[0];
		mins[1] = maxs[1] = surf->verts[0].st[1];
		for (i = 1; i < surf->numverts; i++)
		{
			if (mins[0] > surf->verts[i].st[0])
				mins[0] = surf->verts[i].st[0];
			else if (maxs[0] < surf->verts[i].st[0])
				maxs[0] = surf->verts[i].st[0];
			if (mins[1] > surf->verts[i].st[1])
				mins[1] = surf->verts[i].st[1];
			else if (maxs[1] < surf->verts[i].st[1])
				maxs[1] = surf->verts[i].st[1];
		}
		imins[0] = FastFloor(mins[0]);
		imins[1] = FastFloor(mins[1]);
		imaxs[0] = FastCeil(maxs[0]);
		imaxs[1] = FastCeil(maxs[1]);
		if (imaxs[0] <= imins[0] + 1 && imaxs[1] <= imins[1] + 1)
		{
			if (imins[0] || imins[1])
			{
				for (i = 0; i < surf->numverts; i++)
				{
					surf->verts[i].st[0] -= imins[0];
					surf->verts[i].st[1] -= imins[1];
				}
			}
			continue;
		}
		imins[0] = FastFloor((mins[0] + maxs[0]) * 0.5f);
		imins[1] = FastFloor((mins[1] + maxs[1]) * 0.5f);
		if (imins[0] || imins[1])
		{
			for (i = 0; i < surf->numverts; i++)
			{
				surf->verts[i].st[0] -= imins[0];
				surf->verts[i].st[1] -= imins[1];
			}
			mins[0] -= imins[0];
			maxs[0] -= imins[0];
			mins[1] -= imins[1];
			maxs[1] -= imins[1];
		}
		if (mins[0] >= -snapEpsilon && maxs[0] <= 1.0f + snapEpsilon && mins[1] >= -snapEpsilon &&
			maxs[1] <= 1.0f + snapEpsilon)
		{
			for (i = 0; i < surf->numverts; i++)
			{
				if (surf->verts[i].st[0] < 0)
					surf->verts[i].st[0] = 0;
				else if (surf->verts[i].st[0] > 1)
					surf->verts[i].st[0] = 1;
				if (surf->verts[i].st[1] < 0)
					surf->verts[i].st[1] = 0;
				else if (surf->verts[i].st[1] > 1)
					surf->verts[i].st[1] = 1;
			}
		}
		else if (mins[0] >= -warningEpsilon && maxs[0] <= 1.0f + warningEpsilon &&
				 mins[1] >= -warningEpsilon && maxs[1] <= 1.0f + warningEpsilon)
		{
			printf(
				"*** WARNING: surface %s appears to be accidentally tiling, it probably has vertices too close to the edge\n",
				model.materialNames[surf->material].name);
			surf->state = 1;
		}
		else
		{
			surf->state = 2;
		}
	nextSurface:;
	}
}

/*
=============
SkelModel::VerifyBlendedVertexRoot
=============
*/
void SkelModel::VerifyBlendedVertexRoot() {
	int i;
	int j;
	int root;
	int bone;
	loadvertx_t *vert;
	blendvert_t *blend;

	vert = model.verts;
	for ( i = 0; i < model.numverts; i++, vert++ ) {

		assert( vert->numbones > 0 );
		blend = vert->blend;
		root = blend->bone;
		while ( model.bones[root].parent >= 0 ) root = model.bones[root].parent;
		blend++;
		for ( j = 1; j < vert->numbones; j++, blend++ ) {
			bone = blend->bone;
			while ( model.bones[bone].parent >= 0 ) bone = model.bones[bone].parent;
			if ( root != bone ) Error( "vertex %d attached to 2 different roots\n", i );
		}
	}
}

/*
=============
SkelModel::ReduceBonesPerVertex
=============
Retain the strongest influences supported by the runtime vertex format.
Normalize the retained weights before storing the compact blend list.
Report a vertex whose source weights cannot produce a valid total.
*/
void SkelModel::ReduceBonesPerVertex( loadvertx_t *vert, int vertNum ) {
	loadvertx_t v;
	int j, i;
	float totalweight, maxweight;

	if ( vert->numbones == 0 ) {
		Error( "No valid weights on vertex %d\n", vertNum );
	}

	v = *vert;
	totalweight = 0;
	for ( j = 0; j < v.numbones; j++ ) {
		totalweight += v.blend[j].weight;
	}

	assert( totalweight );

	if ( totalweight != 1 ) {
		for ( j = 0; j < v.numbones; j++ ) {
			v.blend[j].weight *= 1 / totalweight;
		}
	}

	vert->numbones = 0;
	totalweight = 0;

	for ( j = 0; j < v.numbones; j++ ) {

		for ( i = j + 1; i < v.numbones; i++ ) {
			if ( v.blend[j].bone == v.blend[i].bone ) {
				v.blend[j].weight += v.blend[i].weight;
				v.numbones--;
				v.blend[i] = v.blend[v.numbones];
			}
		}
	}

	for ( ;; ) {
		maxweight = 0;

		j = -1;

		for ( i = 0;
			i < v.numbones; i++ ) {
			if ( v.blend[i].bone == -1 ) {
				continue;
			}

			if ( v.blend[i].weight > maxweight ) {
				j = i;
				maxweight = v.blend[j].weight;
			}
		}

		if ( maxweight == 0 ) {
			break;
		}

		assert( j != -1 );

		if ( maxweight < 0.01f ) {
			printf(
				"Bone %d for vertex %d ignored (weight %f too small)\n",
				v.blend[j].bone,
				vertNum,
				maxweight );
			v.blend[j].bone = -1;
			continue;
		}

		vert->blend[vert->numbones].bone = v.blend[j].bone;
		vert->blend[vert->numbones].weight = maxweight;

		totalweight += maxweight;
		v.blend[j].bone = -1;
		vert->numbones++;

		if ( vert->numbones >= 4 ) {
			for ( j = 0; j < v.numbones; j++ ) {
				if ( v.blend[j].bone == -1 ) {
					continue;
				}
				printf( "Bone %d for vertex %d ignored (max blend bones exceeded)\n", v.blend[j].bone, vertNum );
			}
			break;
		}
	}
	assert( totalweight > 0 );
	assert( vert->numbones > 0 );

	if ( totalweight != 1 ) {
		for ( j = 0; j < vert->numbones; j++ ) {
			vert->blend[j].weight /= totalweight;
		}
	}
}

/* Assign the model vertices to one requested rigid root bone. */
void SkelModel::MakeModelRigid( const char *root ) {
	blendvert_t *blend;
	loadvertx_t *vert;
	int i;
	int bone;

	if ( !root[0] ) {

		assert( model.numbones > 0 );

		model.numbones = 1;
		model.bones[0].name[0] = 0;
		bone = 0;
	} else {
		for ( bone = 0; bone < model.numbones; bone++ ) {
			if ( !stricmp( model.bones[bone].name, root ) ) {
				break;
			}
		}

		if ( bone == model.numbones ) {
			Error( "rigid root '%s' not found", root );
		}
	}

	for ( i = 0; i < model.numverts; i++ ) {
		vert = &model.verts[i];

		assert( vert->numbones > 0 );
		vert->numbones = 1;
		blend = &vert->blend[0];
		blend->bone = bone;
		blend->weight = 1;
	}
}


#endif

#ifdef XANIM_CONVERTER
/*
=============
NormalizeFilename
=============
*/
static void NormalizeFilename( char *out, const char *filename, int count ) {
	char c;

	assert( count );
	for ( ;; ) {
		for ( ;; ) {
			c = *filename;
			filename++;
			if ( c == '\\' || c == '/' ) continue;
			break;
		}
		while ( (unsigned char)c >= ' ' ) {
			*out = tolower( c );
			out++;
			if ( !--count ) {
				Com_Error( 1, "Filename '%s' exceeds maximum length of %d", filename, count );
			}
			if ( c == '\\' ) break;
			c = *filename;
			filename++;
			if ( c == '/' ) c = '\\';
		}
		if ( !c ) {
			*out = 0;
			return;
		}
	}
}

#define QUAT_BONE_ENTRY_SIZE 4096
#define QUAT_BONE_LIST_SIZE 256

/*
Each normalized model path selects a contiguous quaternion-override group.
The filename owns its string; bones points into g_quatBoneEntries.
*/
struct QuatBoneList {
	QuatBoneEntry *bones;
	std::string filename;
};

QuatBoneList g_quatBoneList[QUAT_BONE_LIST_SIZE];
QuatBoneEntry g_quatBoneEntries[QUAT_BONE_ENTRY_SIZE];
int g_quatBoneListSize = 0;
int g_quatBoneEntrySize = 0;
int g_quatBoneGroupStart = 0;

/*
=============
AddQuatBoneList
=============
*/
void AddQuatBoneList( const char *filename, bool first ) {
	char newFilename[1024];
	int i;
	SkelModel *model;
	loadbone_t *basePose;
	loadbone_t *pose;
	loadbone_t *parentPose;
	int j;
	Bone *bone;
	int parentIndex;
	float parentMatrixInv[4][3];
	float mat[3][3];
	QuatBoneEntry *entry;

	if ( first ) {
		g_quatBoneGroupStart = g_quatBoneEntrySize;
	}
	assert( g_quatBoneListSize < QUAT_BONE_LIST_SIZE );

	NormalizeFilename( newFilename, filename, sizeof( newFilename ) );
	for ( i = 0; i < g_quatBoneListSize; i++ ) {
		if ( !strcmp( g_quatBoneList[i].filename.c_str(), newFilename ) ) {
			Error( "duplicate filename '%s'", filename );
		}
	}

	// Each model in a match group shares the same bone rotation table.
	g_quatBoneList[i].filename = newFilename;
	g_quatBoneList[i].bones = &g_quatBoneEntries[g_quatBoneGroupStart];
	g_quatBoneListSize++;

	model = 0;
	try {
		model = new SkelModel;
		model->ParseBaseFile( newFilename );
		basePose = model->model.frames[0].bones;
		for ( j = 0; j < model->model.numbones; j++ ) {
			bone = &model->model.bones[j];

			// The first model to name a bone supplies its reference rotation.
			for ( entry = &g_quatBoneEntries[g_quatBoneGroupStart]; entry != &g_quatBoneEntries[g_quatBoneEntrySize]; entry++ ) {
				if ( !stricmp( bone->name, entry->name.c_str() ) ) goto nextBone;
			}

			// Append new names once, preserving the order of the model skeleton.
			// The previous entry marks whether another rotation follows in
			// this match group. Existing names keep their first rotation.

			assert( g_quatBoneEntrySize < QUAT_BONE_ENTRY_SIZE );
			if ( g_quatBoneGroupStart != g_quatBoneEntrySize ) {
				g_quatBoneEntries[g_quatBoneEntrySize - 1].more = true;
			}
			parentIndex = bone->parent;
			pose = &basePose[bone->fileIndex];
			parentPose = &basePose[parentIndex >= 0 ? model->model.bones[parentIndex].fileIndex : bone->fileIndex];
			MatrixTranspose( parentPose->axis, parentMatrixInv );
			MatrixMultiply( pose->axis, parentMatrixInv, mat );
			ConvertMatToQuat( mat, entry->quat );
			entry->name = bone->name;
			entry->more = false;
			g_quatBoneEntrySize++;
		nextBone:;
		}
	} catch ( char *error ) {
		if ( model ) {
			delete model;
		}
		throw error;
	}
	if ( model ) {
		delete model;
	}
}



/*
=============
ConvertAnimation
=============
*/
void ConvertAnimation( AnimConvertInfo *options ) {
	SkelModel *animation = 0;
	SkelModel *basePose = 0;
	SkelModel *matchModel = 0;
	char newFilename[1024];
	try {
		animation = new SkelModel;
		basePose = new SkelModel;
		animation->model.looping = options->looping;
		animation->model.useBones = options->useBones;
		animation->model.rotationThreshold = options->rotationThreshold;
		animation->model.translationThreshold = options->translationThreshold;
		animation->SetBoneStabilizers( options->stabilizers );
		animation->ParseBaseFile( options->modelFilename );
		basePose->CopyBasePose( *animation );
		animation->LoadAnimation( options->animFilename, options->node );
		QuatBoneList *match = 0;
		NormalizeFilename( newFilename, options->modelFilename, sizeof( newFilename ) );
		for ( int i = 0; i < g_quatBoneListSize; i++ ) {
			if ( !strcmp( g_quatBoneList[i].filename.c_str(), newFilename ) ) {
				match = &g_quatBoneList[i];
				break;
			}
		}
		animation->SaveAnimation( options->outputFilename, options->type,
			basePose->model.frames[0].bones, match ? match->bones : 0 );
	} catch ( char *error ) {
		if ( animation ) delete animation;
		if ( basePose ) delete basePose;
		if ( matchModel ) delete matchModel;
		throw error;
	}
	if ( animation ) delete animation;
	if ( basePose ) delete basePose;
	if ( matchModel ) delete matchModel;
}

#endif

#ifndef XANIM_CONVERTER
// Bone classifications accompany the geometry retained for export.
/*
=============
SkelModel::SetPartClassification
=============
Apply the selected hit-location classification to the loaded bones.
Use the classification table associated with this model family.
Unlisted bones inherit their parent classification, or zero at a root.
*/
void SkelModel::SetPartClassification(int classification)
{
	ModelBone *bone;
	int j, i, parent;
	for (i = 0; i < model.numbones; i++)
	{
		bone = &model.bones[i];
		for (j = 0; j < numPartClassifications[classification]; j++)
		{
			if (!stricmp(bone->name,
						 partClassifications[classification][j].name))
				break;
		}
		if (j != numPartClassifications[classification])
		{
			bone->classification =
				partClassifications[classification][j].classification;
			continue;
		}
		parent = bone->parent;
		if (parent != -1)
		{
			bone->classification = model.bones[parent].classification;
			continue;
		}
		bone->classification = 0;
	}
}

// Incident edge ranges bound the subsequent split and smoothing comparisons.
/*
=============
SkelModel::UpdateVertexEdgeLengths
=============
Measure one triangle edge from its two source vertices.
Expand the minimum and maximum incident edge lengths at both ends.
The normal smoothing pass later uses these local ranges.
*/
void SkelModel::UpdateVertexEdgeLengths(loadvertx_t *v1, loadvertx_t *v2)
{
	float length = VectorDistance(v1->pos, v2->pos);
	if (length > v1->maxEdgeLength)
		v1->maxEdgeLength = length;
	if (length > v2->maxEdgeLength)
		v2->maxEdgeLength = length;
	if (length < v1->minEdgeLength)
		v1->minEdgeLength = length;
	if (length < v2->minEdgeLength)
		v2->minEdgeLength = length;
}

/*
=============
SkelModel::MeasureEdgeLengths
=============
Initialize the incident edge ranges on every model vertex.
Visit all three edges of each source triangle.
Repeated visits preserve the same minimum and maximum bounds.
*/
void SkelModel::MeasureEdgeLengths()
{
	int i, j;
	ModelFace *face;
	for (i = 0; i < model.numSourceVerts; i++)
	{
		model.verts[i].maxEdgeLength = 0;
		model.verts[i].minEdgeLength = FLT_MAX;
	}
	face = model.faces;
	for (j = 0; j < model.numfaces; j++, face++)
	{
		UpdateVertexEdgeLengths(&model.verts[face->corners[0].index],
						  &model.verts[face->corners[1].index]);
		UpdateVertexEdgeLengths(&model.verts[face->corners[0].index],
						  &model.verts[face->corners[2].index]);
		UpdateVertexEdgeLengths(&model.verts[face->corners[1].index],
						  &model.verts[face->corners[2].index]);
	}
}

/*
=============
Model::FindMatchingObjectVertex
=============
Find geometrically equivalent vertices owned by another source object.
Mark the retained source vertex when those objects share its position.
The equality check also requires corresponding blend-bone assignments.
*/
void Model::FindMatchingObjectVertex(long faceIndex, long vertexIndex) const
{
	int index;
	ModelFace *face, *other;
	FaceCorner *corner, *faceVert;
	loadvertx_t *vert;
	int i, j, object;

	face = &model.faces[faceIndex];
	corner = &face->corners[vertexIndex];
	index = corner->index;
	vert = &model.verts[index];
	if (vert->merged)
		return;
	object = model.faces[faceIndex].object;
	for (i = 0; i < model.numfaces; i++)
	{
		other = &model.faces[i];
		if (other->object == object)
			continue;
		faceVert = other->corners;
		for (j = 0; j < 3; j++, faceVert++)
		{
			if (!::VertsEqual(&model.verts[faceVert->index], vert, true))
				continue;
			vert->merged = true;
			model.verts[faceVert->index].merged = true;
			return;
		}
	}
}

/*
=============
Model::ConnectFaceVertices
=============
Connect compatible triangle corners in their circular sharing lists.
Merge lists for later corners referencing the same retained vertex.
The smoothing pass can then visit each connected corner group.
*/
void Model::ConnectFaceVertices(unsigned int faceIndex, int vertexIndex) const
{
	int index;
	ModelFace *face, *sourceFace;
	FaceCorner *other, *vert, *next, *prev, *link;
	int i, j, object;

	sourceFace = &model.faces[faceIndex];
	vert = &sourceFace->corners[vertexIndex];
	index = vert->index;
	if (!model.verts[index].merged)
		return;
	object = model.faces[faceIndex].object;
	for (i = faceIndex + 1; i < model.numfaces; i++)
	{
		face = &model.faces[i];
		other = face->corners;
		for (j = 0; j < 3; j++, other++)
		{
			if (other->index != index)
				continue;
			link = other;
			do
			{
				if (link == vert)
					return;
				link = link->next;
			} while (link != other);
			next = vert->next;
			prev = other->prev;
			vert->next = other;
			other->prev = vert;
			prev->next = next;
			next->prev = prev;
			return;
		}
	}
}

/*
=============
Model::FindDuplicateVerts
=============
*/
void Model::FindDuplicateVerts() const {
	int i;

	model.numverts = 0;

	for ( i = 0; i < model.numSourceVerts; i++ ) {
		model.vertRemap[i] = FindVert( &model.verts[i] );
		if ( model.vertRemap[i] != model.numverts ) {
			if ( model.verts[i].merged ) {
				model.verts[model.vertRemap[i]].merged = true;
			}
			continue;
		}

		assert( model.numverts <= i );
		memcpy( &model.verts[model.numverts], &model.verts[i], sizeof( loadvertx_t ) );
		model.numverts++;
	}
}

/*
=============
NormalsAgree
=============
*/
static bool NormalsAgree( FaceCorner *head ) {
	FaceCorner *a, *b;

	a = head;
	do {
		b = a->next;
		while ( b != head ) {
			if ( DotProduct( a->normal, b->normal ) <= 0 ) {
				return false;
			}
			b = b->next;
		}
		a = a->next;
	} while ( a != head );
	return true;
}

/*
=============
Model::SmoothNormals
=============
*/
void Model::SmoothNormals() const
{
	int i, k, j, m;
	ModelFace *face, *other;
	FaceCorner *corner, *c, *next;
	vec3_t normal;
	int v0, v1, v2;
	int count;
	bool changed;

	for (i = 0; i < model.numfaces; i++)
	{
		for (k = 0; k < 3; k++)
		{
			FindMatchingObjectVertex(i, k);
		}
	}
	memcpy(model.vertsSaved, model.verts,
		   model.numSourceVerts * sizeof(loadvertx_t));
	changed = false;
	FindDuplicateVerts();
	for (face = model.faces, i = 0; i < model.numfaces; i++, face++)
	{
		for (k = 0; k < 3; k++)
		{
			v0 = model.vertRemap[face->corners[k].index];
			v1 = model.vertRemap[face->corners[(k + 1) % 3].index];
			v2 = model.vertRemap[face->corners[(k + 2) % 3].index];
			if (!model.verts[v0].merged && !model.verts[v1].merged)
			{
				continue;
			}
			count = 0;
			for (other = model.faces, j = 0; j < model.numfaces; j++, other++)
			{
				for (m = 0; m < 3; m++)
				{
					if ((v0 != model.vertRemap[other->corners[m].index] ||
						 v1 != model.vertRemap[other->corners[(m + 1) % 3]
												   .index]) &&
						(v1 != model.vertRemap[other->corners[m].index] ||
						 v0 !=
							 model
								 .vertRemap[other->corners[(m + 1) % 3].index]))
						continue;
					count++;
					if (i != j &&
						v2 ==
							model.vertRemap[other->corners[(m + 2) % 3].index])
					{
						changed = true;
						model.verts[v0].merged = false;
						model.verts[v1].merged = false;
						model.verts[v2].merged = false;
					}
				}
			}
			if (count > 2)
			{
				changed = true;
				model.verts[v0].merged = false;
				model.verts[v1].merged = false;
			}
		}
	}
	if (changed)
	{
		for (i = 0; i < model.numSourceVerts; i++)
		{
			model.vertsSaved[i].merged = model.verts[model.vertRemap[i]].merged;
		}
		memcpy(model.verts, model.vertsSaved,
			   model.numSourceVerts * sizeof(loadvertx_t));
		FindDuplicateVerts();
	}
	for (face = model.faces, i = 0; i < model.numfaces; i++, face++)
	{
		corner = face->corners;
		for (k = 0; k < 3; k++, corner++)
		{
			corner->index = model.vertRemap[corner->index];
			corner->next = corner;
			corner->prev = corner;
		}
	}
	for (i = 0; i < model.numfaces; i++)
	{
		for (k = 0; k < 3; k++)
		{
			ConnectFaceVertices(i, k);
		}
	}
	for (face = model.faces, i = 0; i < model.numfaces; i++, face++)
	{
		corner = face->corners;
		for (k = 0; k < 3; k++, corner++)
		{
			if (corner->next == corner)
			{
				continue;
			}
			if (NormalsAgree(corner))
			{
				normal[0] = normal[1] = normal[2] = 0;
				c = corner;
				do
				{
					VectorAdd(normal, c->normal, normal);
					c = c->next;
				} while (c != corner);
				VectorNormalize(normal);
				c = corner;
				do
				{
					next = c->next;
					c->next = c;
					VectorCopy(normal, c->normal);
					c = next;
				} while (c != corner);
			}
			else
			{
				c = corner;
				do
				{
					next = c->next;
					c->next = c;
					c = next;
				} while (c != corner);
			}
		}
	}
}

/*
=============
SkelModel::CalcBoneBounds
=============
Build local bounds from the vertices influenced by each bone.
Use the first pose to transform model-space positions into bone space.
Initialize each range at the bone origin when no vertex contributes.
These bounds are retained with the exported skeletal model.
*/
void SkelModel::CalcBoneBounds() {
	int i, vertIndex;
	ModelBone *bone;

	vec3_t mins;
	vec3_t maxs;
	blendvert_t *blend;
	BonePose *pose;

	vec3_t vec;
	vec3_t vec2;
	float matInv[3][3];

	for ( i = 0; i < model.numbones; i++ ) {
		mins[0] = mins[1] = mins[2] = 0;
		maxs[0] = maxs[1] = maxs[2] = 0;

		for ( vertIndex = 0; vertIndex < model.numverts; vertIndex++ ) {

			assert( model.verts[vertIndex].numbones > 0 );
			blend = model.verts[vertIndex].blend;
			if ( blend->bone != i ) continue;
			pose = &model.poses->pose[blend->bone];
			VectorSubtract( model.verts[vertIndex].pos, pose->origin, vec );
			MatrixTranspose( pose->axis, matInv );
			MatrixTransformVector( vec, matInv, vec2 );
			if ( vec2[0] < mins[0] ) mins[0] = vec2[0];
			if ( vec2[0] > maxs[0] ) maxs[0] = vec2[0];
			if ( vec2[1] < mins[1] ) mins[1] = vec2[1];
			if ( vec2[1] > maxs[1] ) maxs[1] = vec2[1];
			if ( vec2[2] < mins[2] ) mins[2] = vec2[2];
			if ( vec2[2] > maxs[2] ) maxs[2] = vec2[2];
		}
		bone = &model.bones[i];
		VectorCopy( mins, bone->mins );
		VectorCopy( maxs, bone->maxs );
	}
}

/*
=============
SkelModel::TransferBoneBounds
=============
*/
void SkelModel::TransferBoneBounds( SkelModel *source ) {
	int i, j;
	const char *name, *boneName;
	ModelBone *sourceBone, *bone;

	for ( j = 0; j < model.numbones; j++ ) {
		bone = &model.bones[j];
		bone->mins[0] = bone->mins[1] = bone->mins[2] = 0;
		bone->maxs[0] = bone->maxs[1] = bone->maxs[2] = 0;
	}

	for ( i = 0; i < source->model.numbones; i++ ) {
		sourceBone = &source->model.bones[i];
		name = sourceBone->name;
		for ( j = 0; j < model.numbones; j++ ) {
			bone = &model.bones[j];
			boneName = bone->name;
			if ( strcmp( name, boneName ) ) continue;
			bone->mins[0] = sourceBone->mins[0];
			bone->mins[1] = sourceBone->mins[1];
			bone->mins[2] = sourceBone->mins[2];
			bone->maxs[0] = sourceBone->maxs[0];
			bone->maxs[1] = sourceBone->maxs[1];
			bone->maxs[2] = sourceBone->maxs[2];
			break;
		}
	}
}

/*
=============
BuildModel
=============
*/
void BuildModel( xmodelInfo_t *lods, int lod, const char *source, const char *parts,
	const char *sourceDir, const char *gameDir, const char *skinDir,
	bool writeOffsets, const char *surfs, const char *controllers, const char *stabilizers,
	modelBoneNames_t *bones, bool radialNormals, const float radialOrigin[3], int animated,
	const char *rigidRoot, skinNames_t *surfNames, skinOverride_t overrides[], int numOverrides,
	bool viewmodel, const char *hitBox, int classification ) {
	SkelModel *model, *hitModel;
	int i;

	model = new SkelModel;
	model->sourceDir = sourceDir;
	model->targetRoot = gameDir;
	model->skinDir = skinDir;
	model->writeOffsets = writeOffsets;
	model->surfsFile = surfs;
	model->SetBoneControllers( controllers );
	model->SetBoneStabilizers( stabilizers );
	model->viewmodel = viewmodel;
	model->radialNormals = radialNormals;
	model->radialOrigin[0] = radialOrigin[0];
	model->radialOrigin[1] = radialOrigin[1];
	model->radialOrigin[2] = radialOrigin[2];
	model->LoadModel( source, numOverrides );
	model->MeasureEdgeLengths();
	model->SmoothNormals();
	if ( !animated ) {
		model->MakeModelRigid( rigidRoot );
	} else if ( rigidRoot[0] ) {
		Error( "rigid root '%s' specified without lod being forced rigid", rigidRoot );
	}
	model->VerifyBlendedVertexRoot();
	model->CalcBoneBounds();
	model->CreateSurfaces();
	model->ComputeVertexNormals();
	model->CheckSurfaceTiling();
	if ( parts ) {
		model->SetPartClassification( classification );
		if ( hitBox[0] ) {
			hitModel = new SkelModel;
			hitModel->SetBoneControllers( controllers );
			hitModel->SetBoneStabilizers( stabilizers );
			hitModel->LoadSkelGeometryFile( hitBox );
			hitModel->MeasureEdgeLengths();
			hitModel->SmoothNormals();
			if ( !animated ) {
				hitModel->MakeModelRigid( rigidRoot );
			}
			hitModel->VerifyBlendedVertexRoot();
			hitModel->CalcBoneBounds();
			model->TransferBoneBounds( hitModel );
			delete hitModel;
		}
		model->WriteModelParts( parts );
	}
	model->PrepareSkinNames( surfNames, overrides, numOverrides );
	model->SaveSurfs( surfs );
	if ( lods->mins[0] > lods->maxs[0] ) {
		model->ComputeBoundingBox( *lods );
	}
	if ( lods->collisionLod == lod ) {
		model->GenerateCollisionModel( collmodel );
	}
	for ( i = 0; i < model->model.numbones; i++ ) {
		bones->parents[i] = model->model.bones[i].parent;
		strcpy( bones->names[i], model->model.bones[i].name );
	}
	bones->count = model->model.numbones;
	delete model;
}


#else

/*
=============
SkelModel::SkelModel
=============
*/
SkelModel::SkelModel() {
	InitModel();
}

/*
=============
SkelModel::~SkelModel
=============
*/
SkelModel::~SkelModel() {
	FreeModel();
}

/*
=============
SkelModel::InitModel
=============
*/
void SkelModel::InitModel() {
	memset( &model, 0, offsetof( AnimationData, file ) );
	model.numControllers = 0;
	model.numStabilizers = 0;
}

/*
=============
SkelModel::FreeModel
=============
*/
void SkelModel::FreeModel() {
	int i;

	for ( i = 0; i < model.numframes; i++ ) {
		delete [] model.frames[i].bones;
	}
	delete [] model.nodeFrames;
	delete [] model.vertRemap;
	delete [] model.verts;
	delete [] model.vertsSaved;
	delete [] model.faces;
	delete [] model.frames;
	delete [] model.bones;
	delete [] model.fileToModel;
	delete [] model.surfaces;
	delete [] model.objectNames;
	delete [] model.materialNames;
	delete [] model.animToModel;
	memset( &model, 0, offsetof( AnimationData, file ) );
}

/*
=============
Model::ParseBaseFile
=============
*/
void Model::ParseBaseFile( char filename[] ) {
	Script script;
	script.LoadFile( filename );
	script.Skip( "MODEL", true );
	script.Skip( "VERSION", true );
	int version = script.GetInteger( false );
	if ( version != 5 ) {
		script.Error( "Expecting version %d but found version %d.\n", 5, version );
	}
	LoadBoneSection( &script );
	ReadBaseBones( &script );
}

/*
=============
SkelModel::SetBoneStabilizers
=============
*/
void SkelModel::SetBoneStabilizers( const char *text ) {
	const char *ptr;
	model.numStabilizers = 0;
	for ( ptr = COM_Parse( text ); ptr; ptr = COM_Parse( ptr ) ) {
		if ( model.numStabilizers >= MAX_BONE_STABILIZERS ) {
			Error( "maximum bone stabilizers exceeded" );
		}
		strcpy( model.stabilizerBone[model.numStabilizers], com_token );
		strlower( model.stabilizerBone[model.numStabilizers] );
		ptr = COM_Parse( ptr );
		if ( !ptr ) {
			Error( "no bone stabilizer name specified for %s", model.stabilizerBone[model.numStabilizers] );
		}
		strcpy( model.stabilizerName[model.numStabilizers], com_token );
		strlower( model.stabilizerName[model.numStabilizers] );
		model.numStabilizers++;
	}
}

/*
=============
SkelModel::LoadAnimation
=============
*/
void SkelModel::LoadAnimation( const char *filename, const char *node ) {
	Script script;
	memset( model.name, 0, sizeof( model.name ) );
	strncpy( model.name, filename, sizeof( model.name ) - 1 );
	script.LoadFile( filename );
	script.Skip( "ANIMATION", true );
	script.Skip( "VERSION", true );
	int version = script.GetInteger( false );
	if ( version != 3 ) {
		script.Error( "Expecting version %d but found version %d.\n", 3, version );
	}
	LoadAnimationParts( script, node );
	LoadFrames( script );
	LoadNoteTracks( script );
}

/*
=============
SkelModel::LoadObjects
=============
*/
void SkelModel::LoadObjects( Script &script ) {
	int i;
	char *object;
	int count;

	script.Skip( "NUMOBJECTS", true );
	count = script.GetInteger( false );
	model.objectNames = new char[count][64];
	memset( model.objectNames, 0, count * sizeof( *model.objectNames ) );
	model.numObjects = 0;
	for ( i = 0; i < count; i++ ) {
		script.Skip( "OBJECT", true );
		object = model.objectNames[model.numObjects];
		script.GetInteger( false );
		memset( object, 0, sizeof( model.objectNames[0] ) );
		strncpy( object, script.GetToken( false ), sizeof( model.objectNames[0] ) - 1 );
		model.numObjects++;
	}
}

/*
=============
Model::LoadBoneSection
=============
*/
void Model::LoadBoneSection( Script *const script ) {
	Bone *bone;
	int i;
	int j;
	int k;
	int number;
	int parent;
	char szName[MODEL_NAME_LEN];

	script->Skip( "NUMBONES", true );
	model.numFileBones = script->GetInteger( false );
	model.numbones = model.numFileBones + model.numControllers + model.numStabilizers;
	if ( model.numbones > MAX_BONES ) {
		script->Error( "Too many bones in model.  Numbones (including bone controllers & bone stabilizers) cannot exceed %d.", MAX_BONES );
	}
	// Allocate space for the exported bones and the generated helper bones.
	// fileToModel follows the export numbering as helpers are inserted.
	model.bones = new Bone[model.numbones];
	model.fileToModel = new int[model.numFileBones];
	for ( i = 0; i < model.numFileBones; i++ ) {
		model.fileToModel[i] = i;
	}
	memset( model.controllerUsed, 0, sizeof( model.controllerUsed ) );
	memset( model.stabilizerUsed, 0, sizeof( model.stabilizerUsed ) );
	// Read the file bones in order. Each export number must agree with
	// its position, and each parent must refer to the original hierarchy.
	// A stabilizer is inserted immediately before its associated bone.
	// A controller is inserted immediately after its associated bone.
	// Both change the file-to-model mapping for subsequent bones.
	// All generated names are normalized just like the exported names.
	// This keeps the animation part lookup independent of filename case.
	// Unused helpers are diagnosed after the entire hierarchy is built.
	bone = model.bones;
	for ( i = 0; i < model.numFileBones; i++, bone++ ) {
		script->Skip( "BONE", true );
		number = script->GetInteger( false );
		if ( number != i ) {
			script->Error( "Bone number %i out of sync", number );
		}
		parent = script->GetInteger( false );
		if ( parent < -1 || parent >= model.numFileBones ) {
			script->Error( "Parent bone out of range for bone %i", number );
		}
		if ( parent != -1 ) {
			parent = model.fileToModel[parent];
		}
		strncpy( szName, script->GetString( false ), MODEL_NAME_LEN - 1 );
		strlower( szName );

		// The generated stabilizer inherits this bone's original parent.
		// The file bone is then reparented to that new stabilizer.
		for ( j = 0; j < model.numStabilizers; j++ ) {
			if ( stricmp( szName, model.stabilizerBone[j] ) ) {
				continue;
			}
			if ( parent == -1 ) {
				script->Error( "Bone stabilizers not allowed on root bone %s", model.stabilizerBone[j] );
			}
			bone->fileIndex = model.fileToModel[i];
			bone->set = false;
			bone->parent = parent;
			memset( bone->name, 0, MODEL_NAME_LEN );
			strncpy( bone->name, model.stabilizerName[j], MODEL_NAME_LEN - 1 );
			bone->stabilizer = 1;
			model.stabilizerUsed[j] = 1;
			for ( k = i; k < model.numFileBones; k++ ) {
				model.fileToModel[k]++;
			}
			parent = bone->fileIndex;
			bone++;
			for ( k = j + 1; k < model.numStabilizers; k++ ) {
				if ( !stricmp( bone->name, model.stabilizerBone[k] ) ) {
					script->Error( "More than one bone stabilizers on %s", model.stabilizerBone[k] );
				}
			}
			break;
		}
		bone->fileIndex = model.fileToModel[i];
		bone->set = false;
		bone->parent = parent;
		bone->stabilizer = 0;
		memset( bone->name, 0, MODEL_NAME_LEN );
		strncpy( bone->name, szName, MODEL_NAME_LEN - 1 );

		// Controllers follow the file bone and use its model index as parent.
		// The bounds are supplied later when the model data is processed.
		for ( j = 0; j < model.numControllers; j++ ) {
			if ( stricmp( bone->name, model.controllerBone[j] ) ) {
				continue;
			}
			for ( k = i; k < model.numFileBones; k++ ) {
				model.fileToModel[k]++;
			}
			bone++;
			bone->fileIndex = model.fileToModel[i];
			bone->set = false;
			bone->parent = bone->fileIndex - 1;
			assert( (bone->parent >= 0) && (bone->parent < model.numbones) );

			bone->stabilizer = 0;
			memset( bone->name, 0, MODEL_NAME_LEN );
			strncpy( bone->name, model.controllerName[j], MODEL_NAME_LEN - 1 );
			strlower( bone->name );
			model.controllerUsed[j] = 1;
			for ( k = j + 1; k < model.numControllers; k++ ) {
				if ( !stricmp( bone->name, model.controllerBone[k] ) ) {
					script->Error( "More than one bone controller on %s", model.controllerBone[k] );
				}
			}
			break;
		}
	}


	// Every requested helper must name an exported bone.
	for ( j = 0; j < model.numControllers; j++ ) {
		if ( model.controllerUsed[j] ) continue;
		Error( "bone '%s' for bone controller '%s' not found", model.controllerBone[j], model.controllerName[j] );
	}

	for ( j = 0; j < model.numStabilizers; j++ ) {
		if ( model.stabilizerUsed[j] ) continue;
		Error( "bone '%s' for bone stabilizer '%s' not found", model.stabilizerBone[j], model.stabilizerName[j] );
	}

	// Controllers and stabilizers also participate in name lookup.
	// Reject collisions after all of them have been inserted.
	bone = model.bones;
	for ( i = 0; i < model.numbones; i++, bone++ ) {
		for ( j = i + 1; j < model.numbones; j++ ) {
			if ( !stricmp( bone->name, model.bones[j].name ) ) {
				Error( "duplicate bone '%s'", bone->name );
			}
		}
	}
}

/*
=============
SkelModel::LoadAnimationParts

Map animated parts to model bones and remove unused bones.
=============
*/
void SkelModel::LoadAnimationParts( Script &script, const char *nodeName ) {
	Bone *bone;
	int i;
	int stabilizer;
	int partNumber;
	const char *partName;
	int bonenum;
	int j;

	// Parts are listed independently of the model skeleton. A part which is
	// absent from the model can still be selected as the motion node.

	script.Skip( "NUMPARTS", true );
	model.numNodeParts = script.GetInteger( false );
	model.numParts = model.numNodeParts + model.numStabilizers;
	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		model.bones[bonenum].used = false;
	}
	model.animToModel = new short[model.numParts];
	model.nodePart = -1;

	// Resolve the exported part names before adding stabilizer parts.

	for ( i = 0; i < model.numNodeParts; i++ ) {
		script.Skip( "PART", true );
		partNumber = script.GetInteger( false );
		if ( partNumber != i ) {
			script.Error( "Part number out of sync" );
		}
		model.animToModel[partNumber] = -1;
		partName = script.GetString( false );
		bone = model.bones;
		for ( bonenum = 0; bonenum < model.numbones; bonenum++, bone++ ) {
			if ( !stricmp( partName, bone->name ) ) {
				if ( bone->used ) {
					script.Error( "Duplicate part \"%s\" in animation export.", partName );
				}
				bone->used = true;
				model.animToModel[partNumber] = bonenum;
				break;
			}
		}

		// The motion node must remain separate from the model hierarchy.
		if ( nodeName[0] ) {
			if ( !stricmp( partName, nodeName ) ) {
				if ( bonenum != model.numbones ) {
					script.Error( "Node part \"%s\" cannot be in the model.", nodeName );
				}
				if ( model.nodePart != -1 ) {
					script.Error( "Duplicate node part \"%s\" in animation export.", partName );
				}
				model.nodePart = partNumber;
			}
		}
	}

	// Stabilizers use their generated bone names, rather than exported parts.
	// They are appended to the part-to-bone map in stabilizer order.

	for ( stabilizer = 0; i < model.numParts && stabilizer < model.numStabilizers; i++, stabilizer++ ) {
		model.animToModel[i] = -1;
		partName = model.stabilizerName[stabilizer];
		bone = model.bones;
		for ( bonenum = 0; bonenum < model.numbones; bonenum++, bone++ ) {
			if ( !stricmp( partName, bone->name ) ) {
				if ( bone->used ) {
					script.Error( "Duplicate bone stabilizer part \"%s\" in animation export.", partName );
				}
				bone->used = true;
				model.animToModel[i] = bonenum;
				break;
			}
		}
	}

	if ( nodeName[0] && model.nodePart == -1 ) {
		Error( "Node part \"%s\" not found.", nodeName );
	}

	// Reparent animated bones to their nearest animated ancestor before
	// compacting the table; intermediate unused bones will disappear.

	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		bone = &model.bones[bonenum];
		while ( bone->parent != -1 ) {
			if ( model.bones[bone->parent].used ) break;
			bone->parent = model.bones[bone->parent].parent;
		}
	}

	// Shift the remaining bones and repair both parent and part indices
	// whenever an unused bone is removed.

	for ( bonenum = 0; bonenum < model.numbones; bonenum++ ) {
		if ( model.bones[bonenum].used ) continue;
		for ( j = bonenum + 1; j < model.numbones; j++ ) {

			bone = &model.bones[j];
			assert( bone->parent != bonenum );
			if ( bone->parent > bonenum ) {
				bone->parent--;
			}
			model.bones[j - 1] = *bone;
		}

		for ( i = 0; i < model.numParts; i++ ) {
			assert( model.animToModel[i] != bonenum );
			if ( model.animToModel[i] > bonenum ) {
				model.animToModel[i]--;
			}
		}
		model.numbones--;
	}
}

/*
=============
Model::ReadBaseBones
=============
*/
void Model::ReadBaseBones( Script *const script ) {
	loadbone_t *parent;
	loadbone_t *bone;
	loadbone_t *originBone = 0;
	int i;
	int number;

	// Allocate one frame for the model reference pose.
	// Unspecified bones are filled from their parent below.

	model.numframes = 1;
	model.frames = new Frame[1];
	model.frames[0].bones = new loadbone_t[model.numbones];

	for ( i = 0; i < model.numbones; i++ ) {
		bone = &model.frames[0].bones[i];
		bone->set = 0;
	}

	// The file bone order is independent of the expanded model skeleton.
	for ( i = 0; i < model.numFileBones; i++ ) {
		script->Skip( "BONE", true );
		number = script->GetInteger( false );
		if ( number != i ) {
			script->Error( "Bone number out of sync" );
		}

		bone = &model.frames[0].bones[model.fileToModel[i]];
		bone->set = 1;

		script->Skip( "OFFSET", true );
		script->GetVector( bone->offset, false );

		script->Skip( "SCALE", true );
		script->GetVector( bone->scale, false );

		script->Skip( "X", true );
		script->GetVector( bone->axis[0], false );

		script->Skip( "Y", true );
		script->GetVector( bone->axis[1], false );

		script->Skip( "Z", true );
		script->GetVector( bone->axis[2], false );
	}

	// Stabilizer rotations follow tag_origin in the reference pose.
	if ( model.numStabilizers ) {
		for ( i = 0; i < model.numbones; i++ ) {
			if ( stricmp( model.bones[i].name, "tag_origin" ) ) {
				continue;
			}
			originBone = &model.frames[0].bones[i];
			break;
		}
		if ( !originBone ) {
			script->Error( "Model does not have a tag_origin bone" );
		}
	}

	// Fill generated bones after reading all file bones.
	for ( i = 0; i < model.numbones; i++ ) {
		bone = &model.frames[0].bones[i];
		if ( bone->set ) {
			continue;
		}

		// A missing file pose requires a following bone to copy.
		assert( i != model.numbones - 1 );
		if ( model.bones[i].stabilizer ) {
			parent = &model.frames[0].bones[model.bones[i].parent];
			VectorCopy( parent->offset, bone->offset );
			VectorCopy( parent->scale, bone->scale );
			AxisCopy( originBone->axis, bone->axis );
		} else {
			parent = &model.frames[0].bones[i + 1];
			VectorCopy( parent->offset, bone->offset );
			VectorCopy( parent->scale, bone->scale );
			AxisCopy( parent->axis, bone->axis );
		}
	}
}

/*
=============
Model::CopyBasePose
=============
*/
void Model::CopyBasePose( SkelModel &source ) {
	loadbone_t *dest;
	const loadbone_t *src;
	int i;
	model.numframes = 1;
	model.numbones = source.model.numbones;
	model.frames = new Frame[1];
	model.frames[0].bones = new loadbone_t[model.numbones];
	for ( i = 0; i < model.numbones; i++ ) {
		dest = &model.frames[0].bones[i];
		src = &source.model.frames[0].bones[i];
		VectorCopy( src->offset, dest->offset );
		VectorCopy( src->scale, dest->scale );
		AxisCopy( src->axis, dest->axis );
	}
}

/*
=============
SkelModel::ComputeVertexNormals
=============
*/
void SkelModel::ComputeVertexNormals() {
	surface_t *surf = model.surfaces;
	vec3_t p;
	float matInv[3][3];
	int i, j;

	// Convert each surface normal using its vertex's first bone.
	// The reference pose supplies the inverse rotation.

	for ( i = 0; i < model.numsurfaces; i++, surf++ ) {
		for ( j = 0; j < surf->numverts; j++ ) {

			// Each vertex must have a bone rotation before transforming its normal.
			assert( model.verts[surf->verts[j].index].numbones > 0 );
			MatrixTranspose( model.frames[0].bones[model.verts[surf->verts[j].index].blend[0].bone].axis, matInv );
			MatrixTransformVector( surf->verts[j].normal, matInv, p );
			VectorCopy( p, surf->verts[j].normal );
		}
	}
}

/*
=============
SkelModel::CheckSurfaceTiling
=============
*/
void SkelModel::CheckSurfaceTiling() {
	surface_t *surf;
	int s, i, j;
	float delta[2], mins[2], maxs[2];
	int imins[2], imaxs[2];
	float snapEpsilon = 0.003f;
	float warningEpsilon = 0.05f;
	float seamEpsilon = 0.01f;

	surf = model.surfaces;
	for ( s = 0; s < model.numsurfaces; s++, surf++ ) {
		surf->state = 0;
		if ( !surf->numverts ) continue;
		for ( i = 1; i < surf->numverts; i++ ) {
			for ( j = 0; j < i; j++ ) {
				if ( !VectorCompareEpsilon( model.verts[surf->verts[i].index].pos, model.verts[surf->verts[j].index].pos ) ) continue;
				delta[0] = (float)fabs( surf->verts[i].st[0] - surf->verts[j].st[0] );
				delta[1] = (float)fabs( surf->verts[i].st[1] - surf->verts[j].st[1] );
				if ( delta[0] < seamEpsilon ) {
					if ( fabs( delta[1] - 1.0f ) < seamEpsilon ) {
						surf->state = 2;
						goto nextSurface;
					}
				} else if ( delta[1] < seamEpsilon ) {
					if ( fabs( delta[0] - 1.0f ) < seamEpsilon ) {
						surf->state = 2;
						goto nextSurface;
					}
				}
			}
		}
		mins[0] = maxs[0] = surf->verts[0].st[0];
		mins[1] = maxs[1] = surf->verts[0].st[1];
		for ( i = 1; i < surf->numverts; i++ ) {
			if ( mins[0] > surf->verts[i].st[0] ) mins[0] = surf->verts[i].st[0];
			else if ( maxs[0] < surf->verts[i].st[0] ) maxs[0] = surf->verts[i].st[0];
			if ( mins[1] > surf->verts[i].st[1] ) mins[1] = surf->verts[i].st[1];
			else if ( maxs[1] < surf->verts[i].st[1] ) maxs[1] = surf->verts[i].st[1];
		}
		imins[0] = FastFloor( mins[0] );
		imins[1] = FastFloor( mins[1] );
		imaxs[0] = FastCeil( maxs[0] );
		imaxs[1] = FastCeil( maxs[1] );
		if ( imaxs[0] <= imins[0] + 1 && imaxs[1] <= imins[1] + 1 ) {
			if ( imins[0] || imins[1] ) {
				for ( i = 0; i < surf->numverts; i++ ) {
					surf->verts[i].st[0] -= imins[0];
					surf->verts[i].st[1] -= imins[1];
				}
			}
			continue;
		}
		imins[0] = FastFloor( (mins[0] + maxs[0]) * 0.5f );
		imins[1] = FastFloor( (mins[1] + maxs[1]) * 0.5f );
		if ( imins[0] || imins[1] ) {
			for ( i = 0; i < surf->numverts; i++ ) {
				surf->verts[i].st[0] -= imins[0];
				surf->verts[i].st[1] -= imins[1];
			}
			mins[0] -= imins[0];
			maxs[0] -= imins[0];
			mins[1] -= imins[1];
			maxs[1] -= imins[1];
		}
		if ( mins[0] >= -snapEpsilon && maxs[0] <= 1.0f + snapEpsilon && mins[1] >= -snapEpsilon && maxs[1] <= 1.0f + snapEpsilon ) {
			for ( i = 0; i < surf->numverts; i++ ) {
				if ( surf->verts[i].st[0] < 0 ) surf->verts[i].st[0] = 0;
				else if ( surf->verts[i].st[0] > 1 ) surf->verts[i].st[0] = 1;
				if ( surf->verts[i].st[1] < 0 ) surf->verts[i].st[1] = 0;
				else if ( surf->verts[i].st[1] > 1 ) surf->verts[i].st[1] = 1;
			}
		} else if ( mins[0] >= -warningEpsilon && maxs[0] <= 1.0f + warningEpsilon && mins[1] >= -warningEpsilon && maxs[1] <= 1.0f + warningEpsilon ) {
			printf( "*** WARNING: surface %s appears to be accidentally tiling, it probably has vertices too close to the edge\n", model.materialNames[surf->material].name );
			surf->state = 1;
		} else {
			surf->state = 2;
		}
	nextSurface: ;
	}
}

/*
=============
SkelModel::VerifyBlendedVertexRoot
=============
*/
void SkelModel::VerifyBlendedVertexRoot() {
	int i, j;
	int root, bone;
	loadvertx_t *vert;
	blendvert_t *blend;

	// All blends of a vertex must belong to the same root hierarchy.
	// Compare roots after following each parent chain.

	vert = model.verts;
	for ( i = 0; i < model.numverts; i++, vert++ ) {
		assert( vert->numbones > 0 );
		blend = vert->blend;
		root = blend->bone;
		while ( model.bones[root].parent >= 0 )
			root = model.bones[root].parent;
		blend++;
		for ( j = 1; j < vert->numbones; j++, blend++ ) {
			bone = blend->bone;
			while ( model.bones[bone].parent >= 0 )
				bone = model.bones[bone].parent;
			if ( root != bone )
				Error( "vertex %d attached to 2 different roots\n", i );
		}
	}
}

#endif
