#ifndef UO_CONVERTER_MODELBUILD_H
#define UO_CONVERTER_MODELBUILD_H
/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Read and build skeletal models, render surfaces and collision surfaces.

extern "C" char com_token[];
extern "C" const char *COM_Parse( const char * );
extern "C" char *strlower( char * );

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "xmodel_model.h"
#include <malloc.h>
#include <wchar.h>
#include <ctype.h>
#include "shaderlib.h"
extern loadcollmodel_t collmodel;
#include "q_round.h"

extern "C" {
float VectorDistance( const float *, const float * );
float VectorNormalize( float * );
int VectorCompareEpsilon( const float *, const float * );
}
bool VertsEqual( loadvertx_t *, loadvertx_t *, int );
extern "C" int PlaneFromPoints( float *, const float *, const float *, const float * );

#include <windows.h>
#include <new>
#include <stdlib.h>
#include "../convert_value.h"
#include "script.h"

#pragma intrinsic(memset, memcpy)

#define VectorSubtract( a, b, c ) ( ( c )[0] = ( a )[0] - ( b )[0], ( c )[1] = ( a )[1] - ( b )[1], ( c )[2] = ( a )[2] - ( b )[2] )
#define VectorAdd( a, b, c ) ( ( c )[0] = ( a )[0] + ( b )[0], ( c )[1] = ( a )[1] + ( b )[1], ( c )[2] = ( a )[2] + ( b )[2] )
#define VectorScale( v, s, o ) ( ( o )[0] = ( v )[0] * ( s ), ( o )[1] = ( v )[1] * ( s ), ( o )[2] = ( v )[2] * ( s ) )
#define VectorMA( v, s, b, o ) ( ( o )[0] = ( v )[0] + ( s ) * ( b )[0], ( o )[1] = ( v )[1] + ( s ) * ( b )[1], ( o )[2] = ( v )[2] + ( s ) * ( b )[2] )
#define DotProduct( a, b ) ( ( a )[0] * ( b )[0] + ( a )[1] * ( b )[1] + ( a )[2] * ( b )[2] )
#define VectorClear( a ) ( ( a )[0] = ( a )[1] = ( a )[2] = 0 )
#define VectorSet( v, x, y, z ) ( ( v )[0] = ( x ), ( v )[1] = ( y ), ( v )[2] = ( z ) )
#define VectorCopy( a, b ) ( ( b )[0] = ( a )[0], ( b )[1] = ( a )[1], ( b )[2] = ( a )[2] )

extern "C" {
void AddPointToBounds( const float *v, float *mins, float *maxs );
float VectorDistanceSquared( const float *p1, const float *p2 );

void Com_BeginParseSession( const char *name );
void Com_EndParseSession();
char *Com_Parse( const char **data_p );
void Com_UngetToken();
int Com_SkipBracedSection( const char **data_p, int startDepth );
void Com_SkipRestOfLine( const char **data_p );
}

extern "C" int LoadFile( const char *filename, void **buffer );
extern "C" int TryLoadFile( const char *filename, void **buffer );




extern ShaderInfo shaderLibrary;



static materialName_t *FindMaterial( const char *name, materialName_t *skins, int count );
static int ResolveVertexBones( surface_t *surf );
static bool NormalsAgree( FaceCorner *head );


#endif
