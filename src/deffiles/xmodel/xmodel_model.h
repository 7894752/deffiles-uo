/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#ifndef XMODEL_MODEL_H
#define XMODEL_MODEL_H

#include <stdio.h>

class Script;

typedef float vec3_t[3];

struct blendvert_t {
	int bone;
	float weight;
};

class ConvertValue;

#define MAX_SKIN_LEN 2048
#define XMODEL_VERSION 14
#define MAX_SKIN_OVERRIDE 64
#define MAX_CLASSIFICATION 64

extern "C" void SafeWrite( FILE *f, const void *buffer, int size );
extern "C" FILE *SafeOpenWrite( const char *filename );
extern "C" void Swap_Init();
extern "C++" void Error( const char *fmt, ... );

extern "C" void MatrixTranspose( const float in[3][3], float out[3][3] );
extern "C" void MatrixTransformVector( float *in, float matrix[3][3], float *out );
extern "C" void MatrixMultiply( const float in1[3][3], const float in2[3][3], float out[3][3] );
extern "C" void MatrixTransformVector43( const float *in, const float matrix[4][3], float *out );

extern ConvertValue *convertValue;

// Index group produced by the triangle strip and triangle list generators.
typedef struct triangleIndexGroup_s {
	unsigned int numIndices;
	unsigned short *indices;
} triangleIndexGroup_t;

int MakeStripGroups( const unsigned short *const indices, unsigned short numIndices,
	triangleIndexGroup_t **const groups, unsigned short *remap, int useVertexCache );
unsigned long GenerateTriangleGroups( const unsigned short indices[], unsigned short numIndices,
	triangleIndexGroup_t *&groups, unsigned short remap[], unsigned short numVerts );

extern "C" int MyAssertHandler( const char *expression, const char *file, int line , ...);
#define assert( exp ) { static char ignore = 0; if ( !( exp ) && !ignore ) { switch ( MyAssertHandler( #exp, __FILE__, __LINE__ ) ) { case 2: ignore = true; break; case 0: __asm { int 3 } break; } } }

// Vertex of the source model.
#define MAX_VERT_BLENDS 16

struct loadvertx_t {
	int numbones;
	blendvert_t blend[MAX_VERT_BLENDS];
	vec3_t pos;
	float minEdgeLength;
	float maxEdgeLength;
	bool merged;
};

// One corner of a source face.
struct FaceCorner {
	int index; // vertex index
	float st[2];
	vec3_t normal;
	FaceCorner *prev; // ring of corners sharing a vertex
	FaceCorner *next;
};

struct ModelFace {
	int object;
	int material;
	int unused;
	FaceCorner corners[3];
	bool bUsed;
	int loadIndex; // first triangle index in the surface, -1 when culled
};

// Bone pose as read from the BONE block.
struct BonePose {
	vec3_t origin;
	vec3_t scale;
	vec3_t axis[3];
	int set;
};

struct BonePoses {
	int number;
	BonePose *pose;
};

struct ModelBone {
	int index;
	int parent;
	int reserved;
	char name[0x40];
	bool used;
	bool set;
	int classification;
	int stabilizer;
	vec3_t mins;
	vec3_t maxs;
};

#define MAX_SURF_TRIS 0x2000
#define MAX_SURF_VERTS 0x1000

struct SurfTri {
	int index[3];
};

struct SurfVert {
	int index; // model vertex
	int bone;
	float st[2];
	vec3_t normal;
};

struct surface_t {
	int object;
	int material;
	int numtris;
	int numverts;
	int numUniqueVerts; // set when the surface is written
	int numStrips;
	int boneIndex; // -1 for a physiqued surface
	int state; // tiling state, written as a byte
	SurfTri tris[MAX_SURF_TRIS];
	SurfVert verts[MAX_SURF_VERTS];

};

#define MAX_TRIANGLES MAX_SURF_TRIS

struct CollTri {
	float plane[4];
	float svec[4];
	float tvec[4];
};

struct CollSurface {
	CollTri tris[MAX_TRIANGLES];
	int numCollTris;
	vec3_t mins;
	vec3_t maxs;
	int boneIdx;
	int contents;
	int surfaceFlags;
};

#define MAX_SURFACES 256
#define MAX_COLL_SURFS 64

typedef struct {
	CollSurface collSurfs[MAX_COLL_SURFS];
	int numCollSurfs;
} loadcollmodel_t;

struct materialName_t {
	char name[0x40];
	bool radialNormals;
	bool copied; // image already copied to the target tree
};

// Working model data.
struct ModelData {
	char name[64];
	int framerate;
	mutable int numverts; // retained-vertex cache size
	int numSourceVerts;
	int numfaces;
	int numParts;
	int numNodeParts;
	int numbones;
	int numRootBones;
	int numframes;
	int numsurfaces;
	int maxsurfaces;
	int numObjects;
	int numMaterials;
	int unusedFaces;
	int *vertRemap;
	loadvertx_t *verts;
	loadvertx_t *vertsSaved;
	ModelFace *faces;
	BonePoses *poses;
	ModelBone *bones;
	surface_t *surfaces;
	char (*objectNames)[0x40];
	materialName_t *materialNames;
	int nodePart;
	BonePose *nodeFrames;
	short *animToModel;
	int *boneRemap;
};




#define MAX_BONE_CONTROLLERS 8
#define MAX_BONE_STABILIZERS 2
#define MODEL_NAME_LEN 64

#define MAX_LODS 3
#define MAX_LOD_NAME_LEN 1024

// An xmodel as listed in the .gdf entry: three LOD parts files, bounds and the collision LOD.
struct ModelLod {
	char filename[MAX_LOD_NAME_LEN];
	float distance;
};

typedef struct xmodelInfo_s {
	ModelLod lods[MAX_LODS];
	float mins[3];
	float maxs[3];
	int collisionLod;
} xmodelInfo_t;

// Surface (skin) names of one LOD.
struct skinNames_t {
	int num;
	char *names[256];
};

struct skinOverride_t {
	char material[MODEL_NAME_LEN];
	char object[MODEL_NAME_LEN];
	char skin[MODEL_NAME_LEN];
	bool used;
};

// Bone hierarchy of the model, reported back to the converter for the next LOD.
struct modelBoneNames_t {
	int parents[256];
	char names[256][MODEL_NAME_LEN];
	int count;
};

extern "C" void SaveXModel( const char *filename, xmodelInfo_t *lods, skinNames_t *surfNames );

// Hit location classification of a bone, loaded from source_data/partclassification.csv
struct PartClassification {
	char name[1024];
	int classification;
};

extern PartClassification partClassifications[2][MAX_CLASSIFICATION];
extern int numPartClassifications[2];


// Shared model state and base-bone pose input.
class Model {
public:
	void ParseBaseFile( char filename[] );

	ModelData model;
	FILE *file;
	const char *sourceDir;
	const char *targetRoot;
	const char *skinDir;
	const char *surfsFile;
	vec3_t radialOrigin;
	bool radialNormals;
	bool writeOffsets;
	bool viewmodel;
	float rotationThreshold;
	float translationThreshold;
	int numControllers;
	char controllerNames[MAX_BONE_CONTROLLERS][MODEL_NAME_LEN]; // bone that gets a controller
	char controllerTargets[MAX_BONE_CONTROLLERS][MODEL_NAME_LEN]; // controller bone to add
	int controllerUsed[MAX_BONE_CONTROLLERS];
	int numStabilizers;
	char stabilizerNames[MAX_BONE_STABILIZERS][MODEL_NAME_LEN];
	char stabilizerTargets[MAX_BONE_STABILIZERS][MODEL_NAME_LEN];
	int stabilizerUsed[MAX_BONE_STABILIZERS];

protected:
	void ReadBaseBones( Script *const script );
	int FindVert( loadvertx_t *vert ) const;
	materialName_t *GetNamedMaterial( char *name );
	void FindDuplicateVerts() const;
	void SmoothNormals() const;
	void ConnectFaceVertices( unsigned int faceIndex, int cornerIndex ) const;
private:
	void FindMatchingObjectVertex( long faceIndex, long cornerIndex ) const;
	friend class SkelModel;
	void LoadBoneSection( Script *const script );
};

class SkelModel : public Model {
public:

	void WriteVector( vec3_t v );

	void WriteBlend( blendvert_t *blend, bool noWeight, vec3_t offset );
	const char *GetImageLanguage( const char *filename, int *language );
	char *StripLanguageFromImage( const char *filename );
	void WriteSurface( surface_t *surf );
	void WriteModelParts( const char *filename );
	materialName_t *AddImage( const char *name );
	void SaveSurfs( const char *filename );

	void InitModel();
	void SetBoneControllers( const char *text );
	void SetBoneStabilizers( const char *text );
	void LoadObjects( Script &script );

	void LoadFaces( Script &script );
	bool VertexCompare( int a, int b );

	int AppendSurfaceTriangle( surface_t *surf, FaceCorner *corners, int object, int material );
	surface_t *GetNewSurface( int object, int material, int bone );
	void ComputeBoundingBox( xmodelInfo_t &info ) const;
	void GetSurfaceParms( const char *shaderName, int *contentFlags, int *surfaceFlags );
	void GenerateCollisionModel( loadcollmodel_t &collmodel );
	void ComputeVertexNormals();
	void CheckSurfaceTiling();
	void ReduceBonesPerVertex( loadvertx_t *vert, int vertNum );
	void MakeModelRigid( const char *rootName );
	void SetPartClassification( int classification );
	void MeasureEdgeLengths();



	void CalcBoneBounds();
	void TransferBoneBounds( SkelModel *source );
	bool MaterialUsesRadialNormals( const char *material );
	void LoadMaterials( Script &script, int numExtra );
	void LoadVerts( Script &script );
	SkelModel();
	~SkelModel();
	void FreeModel();
	void LoadAnimationParts( Script &script, const char *nodeName );
	void LoadModel( const char *filename, int numExtra );
	void CreateSurface( int object, int mtlID );
	void CreateSurfaces();
protected:
	void CopySurfaceSkinFiles( const surface_t *const surf );
	void CopyTextureFile( const char *const srcFilename, char *destFilename );
	void PrepareSkinNames( skinNames_t *const surfNames, skinOverride_t *const overrides, long numOverrides );
	void LoadObjectData( Script &script, int numExtra );
	void LoadSkelGeometryFile( const char *filename );
	void ParseExportSection( Script &script );
	void VerifyBlendedVertexRoot();
private:
	friend void BuildModel( xmodelInfo_t *lods, int lod, const char *source, const char *parts,
	const char *sourceDir, const char *gameDir, const char *skinDir,
	bool writeOffsets, const char *surfs, const char *controllers, const char *stabilizers,
	modelBoneNames_t *bones, bool radialNormals, const float radialOrigin[3], int animated,
	const char *rigidRoot, skinNames_t *surfNames, skinOverride_t overrides[], int numOverrides,
	bool viewmodel, const char *hitBox, int classification );
	void UpdateVertexEdgeLengths( loadvertx_t *a, loadvertx_t *b );
	int AppendSurfaceVert( surface_t *const surf, FaceCorner *corner, unsigned int object, int material );
	void WriteQuatXComponent( const short *q );
	void WriteQuatXYZ( const short (&q)[4] );

};

#endif
