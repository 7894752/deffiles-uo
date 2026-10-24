/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Bone hierarchy, animation frames and conversion options.

#ifndef XANIM_MODEL_H
#define XANIM_MODEL_H

#include <stdio.h>
#include "script.h"

extern "C" int MyAssertHandler( const char *expression, const char *file, int line , ...);
#ifdef assert
#undef assert
#endif
#define assert( exp ) { static char ignore = 0; if ( !( exp ) && !ignore ) { switch ( MyAssertHandler( #exp, __FILE__, __LINE__ ) ) { case 2: ignore = true; break; case 0: __asm { int 3 } break; } } }

#define MAX_BONES				256
#define MAX_NOTETRACK_CHANNELS	256
#define MAX_NOTETRACK_FRAMES	32
#define MAX_BONE_CONTROLLERS	8
#define MAX_BONE_STABILIZERS	2
#define NOTETRACK_NAME_LEN		128
#define MODEL_NAME_LEN			64

typedef float vec3_t[3];

#define VectorCopy( a, b )	( ( b )[0] = ( a )[0], ( b )[1] = ( a )[1], ( b )[2] = ( a )[2] )
#define AxisCopy(a,b) ( VectorCopy((a)[0],(b)[0]), VectorCopy((a)[1],(b)[1]), VectorCopy((a)[2],(b)[2]) )
#define VectorClear( a ) ( ( a )[0] = ( a )[1] = ( a )[2] = 0 )

struct Bone {
	int               fileIndex;                           // index of this bone in the export file
	int               parent;
	int               reserved;
	char              name[MODEL_NAME_LEN];
	bool              used;                                // referenced by a part
	bool              set;
	int               classification;
	int               stabilizer;                          // created for a bone stabilizer
	vec3_t            mins;
	vec3_t            maxs;
};

struct loadbone_t {
	vec3_t            offset;
	vec3_t            scale;
	vec3_t            axis[3];
	int               set;
};

struct Frame {
	int               number;
	loadbone_t         *bones;
};

struct NotetrackChannel {
	char              name[NOTETRACK_NAME_LEN];
	int               frames[MAX_NOTETRACK_FRAMES];
	int               numFrames;
};

struct loadvertx_t;
struct ModelFace;
struct surface_t;
struct materialName_t;

struct AnimationData {
	char              name[MODEL_NAME_LEN];
	int               framerate;
	int               numverts;
	int               numSourceVerts;
	int               numfaces;
	int               numParts;                            // node parts plus bone stabilizer parts
	int               numNodeParts;
	int               numbones;                            // including controllers and stabilizers
	int               numFileBones;
	int               numframes;
	int               numsurfaces;
	int               maxsurfaces;
	int               numObjects;
	int               numMaterials;
	int               unusedFaces;
	int               *vertRemap;
	loadvertx_t         *verts;
	loadvertx_t         *vertsSaved;
	ModelFace         *faces;
	Frame             *frames;
	Bone              *bones;
	surface_t      *surfaces;
	char              (*objectNames)[MODEL_NAME_LEN];
	materialName_t      *materialNames;
	int               nodePart;
	loadbone_t         *nodeFrames;
	short             *animToModel;                        // part index -> model bone
	int               *fileToModel;                        // export file bone -> model bone
	FILE              *file;
	const char        *sourceDir;
	const char        *targetRoot;
	const char        *skinDir;
	const char        *surfsFile;
	vec3_t            radialOrigin;
	bool              radialNormals;
	bool              looping;
	bool              useBones;
	float             rotationThreshold;
	float             translationThreshold;
	NotetrackChannel  channels[MAX_NOTETRACK_CHANNELS];
	int               numChannels;
	int               numControllers;
	char              controllerBone[MAX_BONE_CONTROLLERS][MODEL_NAME_LEN];
	char              controllerName[MAX_BONE_CONTROLLERS][MODEL_NAME_LEN];
	int               controllerUsed[MAX_BONE_CONTROLLERS];
	int               numStabilizers;
	char              stabilizerBone[MAX_BONE_STABILIZERS][MODEL_NAME_LEN];
	char              stabilizerName[MAX_BONE_STABILIZERS][MODEL_NAME_LEN];
	int               stabilizerUsed[MAX_BONE_STABILIZERS];
};

class QuatBoneEntry {
public:
	std::string       name;
	float             quat[4];
	bool              more;
};

struct blendvert_t;

class SkelModel;

class Model {
public:
	void ParseBaseFile( char filename[] );
	void CopyBasePose( SkelModel &source );
	AnimationData model;
protected:
	void ReadBaseBones( Script *const script );
private:
	void LoadBoneSection( Script *const script );
};

class SkelModel : public Model {
public:
	SkelModel();
	~SkelModel();

	void	InitModel();
	void	FreeModel();
	void	SetBoneStabilizers( const char *text );
	void	LoadAnimationParts( Script &script, const char *nodeName );
	void	LoadFrames( Script &script );
	void	AddNoteTrack( const char *name, int frame );
	int		GetFrameIndex( int number );
	void	LoadNoteTracks( Script &script );
	void	LoadAnimation( const char *filename, const char *nodeName );

	// Animation export writer.
	void	WriteVector( float v[3] );
	void	WriteBlend( blendvert_t *blend, bool noWeight, vec3_t offset );

	void	WriteQuatX( short iQuat[2] );
	void	SaveNoteTracks();
	bool	SaveQuats( float (*_quatFrames)[4], bool bKeepIdentity );
	void	SaveTrans( float (*trans)[3] );
	void	WriteQuat( float _quatFrames[][2] );
	void	SaveAnimation( const char *filename, int type, loadbone_t *baseFrames, QuatBoneEntry *rotations );

	void LoadObjects(Script &script);
	void ComputeVertexNormals();
	void CheckSurfaceTiling();
protected:
	void VerifyBlendedVertexRoot();

private:
	void WriteQuatXYZ( const short (&iQuat)[4] );

};

extern "C" char com_token[];
extern "C" const char *COM_Parse( const char *text );
extern "C" char *strlower( char *s );
#ifdef __cplusplus
extern "C++"
#endif
void Error( const char *fmt, ... );
void		*Z_Malloc( int size );
void		Z_Free( void *ptr );

struct AnimConvertInfo {
	char              modelFilename[1024];
	char              animFilename[1024];
	char              outputFilename[1024];
	bool              looping;
	bool              useBones;
	int               type;
	char              node[1024];
	float             rotationThreshold;
	float             translationThreshold;
	char              stabilizers[2048];
};

void ConvertAnimation( AnimConvertInfo *options );
void AddQuatBoneList( const char *filename, bool first );

extern "C" {
void AddPointToBounds( const float *v, float *mins, float *maxs );
float VectorDistance( const float *p1, const float *p2 );
float VectorDistanceSquared( const float *p1, const float *p2 );
float VectorNormalize( float *v );
int VectorCompareEpsilon( const float *, const float * );
void MatrixTransformVector( const float *v, const float axis[3][3], float *out );
void Com_Error( int code, const char *format, ... );
void MatrixTranspose( const float in[3][3], float out[3][3] );
void MatrixMultiply( const float a[3][3], const float b[3][3], float out[3][3] );
}
void ConvertMatToQuat( float axis[3][3], float quat[4] );

#endif
