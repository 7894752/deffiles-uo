/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

// Animation export writer and vector utilities.

#ifndef XANIM_H
#define XANIM_H

#include "model_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <string>

extern "C" {
#include "cmdlib.h"
}

#include "xanim_model.h"

#define MAX_NOTETRACK 255

#define Vector2Copy( a, b ) ( (b)[0] = (a)[0], (b)[1] = (a)[1] )
#define Vector4Copy( a, b ) ( (b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2], (b)[3] = (a)[3] )
#define VectorScale( v, s, out ) ( (out)[0] = (v)[0] * (s), (out)[1] = (v)[1] * (s), (out)[2] = (v)[2] * (s) )
#define VectorSubtract( a, b, out ) ( (out)[0] = (a)[0] - (b)[0], (out)[1] = (a)[1] - (b)[1], (out)[2] = (a)[2] - (b)[2] )

// Assertion handling.
#ifdef assert
#undef assert
#endif
extern "C" int MyAssertHandler( const char *expression, const char *file, int line , ...);

#define assert( x ) \
	{ \
		static char ignoreAssert = 0; \
		if ( !( x ) && !ignoreAssert ) { \
			switch ( MyAssertHandler( #x, __FILE__, __LINE__ ) ) { \
			case 2: \
				ignoreAssert = true; \
				break; \
			case 0: \
				__asm { int 3 } \
				break; \
			} \
		} \
	}

extern "C" {
float VectorNormalize4D( float *q );
float VectorDistanceSquared( const float *a, const float *b );
float VectorNormalize2D( float *v );
void MatrixTranspose( const float in[3][3], float out[3][3] );
void MatrixMultiply( const float in1[3][3], const float in2[3][3], float out[3][3] );
void MatrixTransformVector( const float v[3], const float m[3][3], float out[3] );
void MatrixTransformVector43( const float v[3], const float m[4][3], float out[3] );
void Swap_Init();
float AngleEigenTrace( float degrees );
float QuatRatioEigenTrace( const float *a, const float *b );
}

#include "q_endian.h"

struct noteTrack_t {
	char name[NOTETRACK_NAME_LEN];
	int frameIndex;
};

#endif
