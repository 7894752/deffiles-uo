/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

/* Shared vector, matrix, angle and geometric operations. */

#include "com_math.h"

int MyAssertHandler(const char *, const char *, int, ...);
#define Q_ASSERT(expression, message) \
    { \
        static char s_bIgnoreAssert = 0; \
        if (!(expression) && !s_bIgnoreAssert) { \
            switch (MyAssertHandler((message), __FILE__, __LINE__)) { \
            case 2: s_bIgnoreAssert = 1; break; \
            case 0: __asm { int 3 } break; \
            } \
        } \
    }
#define IW_ASSERT(expression) Q_ASSERT(expression, #expression)

#ifdef _WIN32
/* Round float intermediates to their declared precision. */
#pragma optimize("p", on)
#endif

const vec3_t vec3_origin = {0, 0, 0};
vec3_t bytedirs[NUMVERTEXNORMALS] =
{
	{  -0.525731f,   0.000000f,   0.850651f },
	{  -0.442863f,   0.238856f,   0.864188f },
	{  -0.295242f,   0.000000f,   0.955423f },
	{  -0.309017f,   0.500000f,   0.809017f },
	{  -0.162460f,   0.262866f,   0.951056f },
	{   0.000000f,   0.000000f,   1.000000f },
	{   0.000000f,   0.850651f,   0.525731f },
	{  -0.147621f,   0.716567f,   0.681718f },
	{   0.147621f,   0.716567f,   0.681718f },
	{   0.000000f,   0.525731f,   0.850651f },
	{   0.309017f,   0.500000f,   0.809017f },
	{   0.525731f,   0.000000f,   0.850651f },
	{   0.295242f,   0.000000f,   0.955423f },
	{   0.442863f,   0.238856f,   0.864188f },
	{   0.162460f,   0.262866f,   0.951056f },
	{  -0.681718f,   0.147621f,   0.716567f },
	{  -0.809017f,   0.309017f,   0.500000f },
	{  -0.587785f,   0.425325f,   0.688191f },
	{  -0.850651f,   0.525731f,   0.000000f },
	{  -0.864188f,   0.442863f,   0.238856f },
	{  -0.716567f,   0.681718f,   0.147621f },
	{  -0.688191f,   0.587785f,   0.425325f },
	{  -0.500000f,   0.809017f,   0.309017f },
	{  -0.238856f,   0.864188f,   0.442863f },
	{  -0.425325f,   0.688191f,   0.587785f },
	{  -0.716567f,   0.681718f,  -0.147621f },
	{  -0.500000f,   0.809017f,  -0.309017f },
	{  -0.525731f,   0.850651f,   0.000000f },
	{   0.000000f,   0.850651f,  -0.525731f },
	{  -0.238856f,   0.864188f,  -0.442863f },
	{   0.000000f,   0.955423f,  -0.295242f },
	{  -0.262866f,   0.951056f,  -0.162460f },
	{   0.000000f,   1.000000f,   0.000000f },
	{   0.000000f,   0.955423f,   0.295242f },
	{  -0.262866f,   0.951056f,   0.162460f },
	{   0.238856f,   0.864188f,   0.442863f },
	{   0.262866f,   0.951056f,   0.162460f },
	{   0.500000f,   0.809017f,   0.309017f },
	{   0.238856f,   0.864188f,  -0.442863f },
	{   0.262866f,   0.951056f,  -0.162460f },
	{   0.500000f,   0.809017f,  -0.309017f },
	{   0.850651f,   0.525731f,   0.000000f },
	{   0.716567f,   0.681718f,   0.147621f },
	{   0.716567f,   0.681718f,  -0.147621f },
	{   0.525731f,   0.850651f,   0.000000f },
	{   0.425325f,   0.688191f,   0.587785f },
	{   0.864188f,   0.442863f,   0.238856f },
	{   0.688191f,   0.587785f,   0.425325f },
	{   0.809017f,   0.309017f,   0.500000f },
	{   0.681718f,   0.147621f,   0.716567f },
	{   0.587785f,   0.425325f,   0.688191f },
	{   0.955423f,   0.295242f,   0.000000f },
	{   1.000000f,   0.000000f,   0.000000f },
	{   0.951056f,   0.162460f,   0.262866f },
	{   0.850651f,  -0.525731f,   0.000000f },
	{   0.955423f,  -0.295242f,   0.000000f },
	{   0.864188f,  -0.442863f,   0.238856f },
	{   0.951056f,  -0.162460f,   0.262866f },
	{   0.809017f,  -0.309017f,   0.500000f },
	{   0.681718f,  -0.147621f,   0.716567f },
	{   0.850651f,   0.000000f,   0.525731f },
	{   0.864188f,   0.442863f,  -0.238856f },
	{   0.809017f,   0.309017f,  -0.500000f },
	{   0.951056f,   0.162460f,  -0.262866f },
	{   0.525731f,   0.000000f,  -0.850651f },
	{   0.681718f,   0.147621f,  -0.716567f },
	{   0.681718f,  -0.147621f,  -0.716567f },
	{   0.850651f,   0.000000f,  -0.525731f },
	{   0.809017f,  -0.309017f,  -0.500000f },
	{   0.864188f,  -0.442863f,  -0.238856f },
	{   0.951056f,  -0.162460f,  -0.262866f },
	{   0.147621f,   0.716567f,  -0.681718f },
	{   0.309017f,   0.500000f,  -0.809017f },
	{   0.425325f,   0.688191f,  -0.587785f },
	{   0.442863f,   0.238856f,  -0.864188f },
	{   0.587785f,   0.425325f,  -0.688191f },
	{   0.688191f,   0.587785f,  -0.425325f },
	{  -0.147621f,   0.716567f,  -0.681718f },
	{  -0.309017f,   0.500000f,  -0.809017f },
	{   0.000000f,   0.525731f,  -0.850651f },
	{  -0.525731f,   0.000000f,  -0.850651f },
	{  -0.442863f,   0.238856f,  -0.864188f },
	{  -0.295242f,   0.000000f,  -0.955423f },
	{  -0.162460f,   0.262866f,  -0.951056f },
	{   0.000000f,   0.000000f,  -1.000000f },
	{   0.295242f,   0.000000f,  -0.955423f },
	{   0.162460f,   0.262866f,  -0.951056f },
	{  -0.442863f,  -0.238856f,  -0.864188f },
	{  -0.309017f,  -0.500000f,  -0.809017f },
	{  -0.162460f,  -0.262866f,  -0.951056f },
	{   0.000000f,  -0.850651f,  -0.525731f },
	{  -0.147621f,  -0.716567f,  -0.681718f },
	{   0.147621f,  -0.716567f,  -0.681718f },
	{   0.000000f,  -0.525731f,  -0.850651f },
	{   0.309017f,  -0.500000f,  -0.809017f },
	{   0.442863f,  -0.238856f,  -0.864188f },
	{   0.162460f,  -0.262866f,  -0.951056f },
	{   0.238856f,  -0.864188f,  -0.442863f },
	{   0.500000f,  -0.809017f,  -0.309017f },
	{   0.425325f,  -0.688191f,  -0.587785f },
	{   0.716567f,  -0.681718f,  -0.147621f },
	{   0.688191f,  -0.587785f,  -0.425325f },
	{   0.587785f,  -0.425325f,  -0.688191f },
	{   0.000000f,  -0.955423f,  -0.295242f },
	{   0.000000f,  -1.000000f,   0.000000f },
	{   0.262866f,  -0.951056f,  -0.162460f },
	{   0.000000f,  -0.850651f,   0.525731f },
	{   0.000000f,  -0.955423f,   0.295242f },
	{   0.238856f,  -0.864188f,   0.442863f },
	{   0.262866f,  -0.951056f,   0.162460f },
	{   0.500000f,  -0.809017f,   0.309017f },
	{   0.716567f,  -0.681718f,   0.147621f },
	{   0.525731f,  -0.850651f,   0.000000f },
	{  -0.238856f,  -0.864188f,  -0.442863f },
	{  -0.500000f,  -0.809017f,  -0.309017f },
	{  -0.262866f,  -0.951056f,  -0.162460f },
	{  -0.850651f,  -0.525731f,   0.000000f },
	{  -0.716567f,  -0.681718f,  -0.147621f },
	{  -0.716567f,  -0.681718f,   0.147621f },
	{  -0.525731f,  -0.850651f,   0.000000f },
	{  -0.500000f,  -0.809017f,   0.309017f },
	{  -0.238856f,  -0.864188f,   0.442863f },
	{  -0.262866f,  -0.951056f,   0.162460f },
	{  -0.864188f,  -0.442863f,   0.238856f },
	{  -0.809017f,  -0.309017f,   0.500000f },
	{  -0.688191f,  -0.587785f,   0.425325f },
	{  -0.681718f,  -0.147621f,   0.716567f },
	{  -0.442863f,  -0.238856f,   0.864188f },
	{  -0.587785f,  -0.425325f,   0.688191f },
	{  -0.309017f,  -0.500000f,   0.809017f },
	{  -0.147621f,  -0.716567f,   0.681718f },
	{  -0.425325f,  -0.688191f,   0.587785f },
	{  -0.162460f,  -0.262866f,   0.951056f },
	{   0.442863f,  -0.238856f,   0.864188f },
	{   0.162460f,  -0.262866f,   0.951056f },
	{   0.309017f,  -0.500000f,   0.809017f },
	{   0.147621f,  -0.716567f,   0.681718f },
	{   0.000000f,  -0.525731f,   0.850651f },
	{   0.425325f,  -0.688191f,   0.587785f },
	{   0.587785f,  -0.425325f,   0.688191f },
	{   0.688191f,  -0.587785f,   0.425325f },
	{  -0.955423f,   0.295242f,   0.000000f },
	{  -0.951056f,   0.162460f,   0.262866f },
	{  -1.000000f,   0.000000f,   0.000000f },
	{  -0.850651f,   0.000000f,   0.525731f },
	{  -0.955423f,  -0.295242f,   0.000000f },
	{  -0.951056f,  -0.162460f,   0.262866f },
	{  -0.864188f,   0.442863f,  -0.238856f },
	{  -0.951056f,   0.162460f,  -0.262866f },
	{  -0.809017f,   0.309017f,  -0.500000f },
	{  -0.864188f,  -0.442863f,  -0.238856f },
	{  -0.951056f,  -0.162460f,  -0.262866f },
	{  -0.809017f,  -0.309017f,  -0.500000f },
	{  -0.681718f,   0.147621f,  -0.716567f },
	{  -0.681718f,  -0.147621f,  -0.716567f },
	{  -0.850651f,   0.000000f,  -0.525731f },
	{  -0.688191f,   0.587785f,  -0.425325f },
	{  -0.587785f,   0.425325f,  -0.688191f },
	{  -0.425325f,   0.688191f,  -0.587785f },
	{  -0.425325f,  -0.688191f,  -0.587785f },
	{  -0.587785f,  -0.425325f,  -0.688191f },
	{  -0.688191f,  -0.587785f,  -0.425325f },
};
int com_randSeed = 0x89ABCDEF;

/*
=============
Q_rand
=============
*/
int Q_rand( int *seed ) {
	*seed = (69069 * *seed + 1);
	return *seed;
}

/*
=============
Q_random
=============
*/
float Q_random( int *seed ) {
	return ( Q_rand( seed ) & 0xffff ) / (float)0x10000;
}

/*
=============
Q_crandom
=============
*/
float Q_crandom( int *seed ) {
	return 2.0f * ( Q_random( seed ) - 0.5f );
}

/*
=============
gunrandom
=============
*/
void gunrandom( float *outX, float *outY ) {
	float degrees, radius, sinT, cosT;
	degrees = (float)rand() / 32768.0f * 360.0f;
	radius = (float)rand() / 32768.0f;
	FastSinCos( degrees * M_PI / 180.0f, &sinT, &cosT );
	*outX = radius * cosT;
	*outY = radius * sinT;
}

/*
=============
FastSinCos
=============
*/
__inline void FastSinCos( float radians, float *sinOut, float *cosOut ) {
	__asm {
		push eax
		push ecx
		fld radians
		fsincos
		mov ecx, cosOut
		mov eax, sinOut
		fstp dword ptr [ecx]
		fstp dword ptr [eax]
		pop ecx
		pop eax
	}
}

/*
=============
Q_log2
=============
*/
int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val>>=1 ) != 0 ) {
		answer++;
	}
	return answer;
}

/*
=============
Q_acos
=============
*/
float Q_acos( float c ) {
	float angle;

	angle = acos(c);

	if (angle > M_PI) {
		return (float)M_PI;
	}
	if (angle < -M_PI) {
		return (float)M_PI;
	}
	return angle;
}

/*
=============
ClampChar
=============
*/
signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

/*
=============
ClampShort
=============
*/
signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return i;
}

/*
=============
DirToByte
=============
*/
byte DirToByte( vec3_t dir ) {
	byte i, best;
	float d, bestd;

	if ( !dir ) {
		return 0;
	}

	bestd = 0;
	best = 0;
	for ( i = 0 ; i < NUMVERTEXNORMALS ; i++ ) {
		d = DotProduct( dir, bytedirs[i] );
		if ( d > bestd ) {
			bestd = d;
			best = i;
		}
	}

	return best;
}

/*
=============
ByteToDir
=============
*/
void ByteToDir( int b, vec3_t dir ) {
	if ( b < 0 || b >= NUMVERTEXNORMALS ) {
		VectorCopy( vec3_origin, dir );
		return;
	}
	VectorCopy (bytedirs[b], dir);
}

/*
=============
_DotProduct
=============
*/
vec_t _DotProduct (vec3_t v1, vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

/*
=============
_VectorSubtract
=============
*/
void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out)
{
	out[0] = va[0]-vb[0];
	out[1] = va[1]-vb[1];
	out[2] = va[2]-vb[2];
}

/* Compute both values with the same angle reduction. */
/*
=============
_VectorAdd
=============
*/
void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out)
{
	out[0] = va[0]+vb[0];
	out[1] = va[1]+vb[1];
	out[2] = va[2]+vb[2];
}

/*
=============
_VectorCopy
=============
*/
void _VectorCopy (vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

/*
=============
_VectorScale
=============
*/
void _VectorScale (vec3_t v, vec_t scale, vec3_t out)
{
	out[0] = v[0] * scale;
	out[1] = v[1] * scale;
	out[2] = v[2] * scale;
}

/*
=============
_VectorMA
=============
*/
void _VectorMA( const vec3_t a, float scale, const vec3_t b, vec3_t out ) {
	out[0] = a[0] + scale*b[0];
	out[1] = a[1] + scale*b[1];
	out[2] = a[2] + scale*b[2];
}

/*
=============
VectorCompareEpsilon
=============
*/
qboolean VectorCompareEpsilon( const vec3_t p1, const vec3_t p2 ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( ( p1[i] - p2[i] ) * ( p1[i] - p2[i] ) > 0.001f * 0.001f ) {
			return 0;
		}
	}
	return 1;
}

/*
=============
_VectorLength
=============
*/
float _VectorLength( const vec3_t v ) {
	return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

/*
=============
VectorDistance
=============
*/
float VectorDistance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t dir;

	VectorSubtract( p2, p1, dir );
	return (float)sqrt( DotProduct( dir, dir ) );
}

/*
=============
VectorDistanceSquared
=============
*/
float VectorDistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t v;

	VectorSubtract( p2, p1, v );
	return DotProduct( v, v );
}

/*
=============
VectorDistance2D
=============
*/
float VectorDistance2D( const vec3_t p1, const vec3_t p2 ) {
	float dir[2];

	Vector2Subtract( p2, p1, dir );
	return (float)sqrt( dir[0] * dir[0] + dir[1] * dir[1] );
}

/*
=============
VectorDistanceSquared2D
=============
*/
float VectorDistanceSquared2D( const vec3_t p1, const vec3_t p2 ) {
	float v[2];

	Vector2Subtract( p2, p1, v );
	return v[0] * v[0] + v[1] * v[1];
}

/*
=============
CrossProduct
=============
*/
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

/*
=============
CrossProductUp
=============
*/
void CrossProductUp( const vec3_t in, vec3_t out ) {
	out[0] = in[1];
	out[1] = -in[0];
	out[2] = 0;
}

/*
=============
VectorNormalize
=============
*/
vec_t VectorNormalize( vec3_t v ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = (float)sqrt( length );

	if ( length ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
	return length;
}

/*
=============
VectorNormalize2D
=============
*/
float VectorNormalize2D( float v[2] ) {
	float length, ilength;
	length = v[0] * v[0] + v[1] * v[1];
	length = (float)sqrt( length );
	if ( length != 0.0f ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
	}
	return length;
}

/*
=============
VectorNormalize4D
=============
*/
float VectorNormalize4D( vec4_t v ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
	length = (float)sqrt( length );
	if ( length != 0.0f ) {
		ilength = 1.0f / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
		v[3] *= ilength;
	}
	return length;
}

/*
=============
VectorNormalizeFast
=============
*/
void VectorNormalizeFast( vec3_t v ) {
	float ilength;

	ilength = Q_rsqrt( DotProduct( v, v ) );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

/*
=============
Q_rsqrt

Fast inverse square root.
One Newton iteration refines the exponent estimate.
=============
*/
__inline float Q_rsqrt( float number ) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = *( long * ) &y;                        // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
	y  = *( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration

	return y;
}

/*
=============
VectorNormalize2
=============
*/
float VectorNormalize2( const vec3_t v, vec3_t out ) {
	float length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt( length );

	if ( length ) {
		ilength = 1 / length;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	} else {
		VectorClear( out );
	}

	return length;

}

/*
=============
VectorInverse
=============
*/
void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

/*
=============
Vector4Scale
=============
*/
void Vector4Scale( const vec4_t in, float scale, vec4_t out ) {
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
	out[3] = in[3]*scale;
}

/*
=============
VectorMax
=============
*/
float VectorMax( const vec3_t v ) {
	float m;

	m = ( v[0] >= v[1] ) ? v[0] : v[1];
	return ( m >= v[2] ) ? m : v[2];
}

/*
=============
VectorRotate
=============
*/
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out )
{
	IW_ASSERT( in != out );
	out[0] = DotProduct( in, matrix[0] );
	out[1] = DotProduct( in, matrix[1] );
	out[2] = DotProduct( in, matrix[2] );
}

/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							 float degrees ) {
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	int	i;
	vec3_t vr, vup, vf;
	float	rad;

	IW_ASSERT( dir[0] || dir[1] || dir[2] );

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	rad = DEG2RAD( degrees );

	IW_ASSERT( !IS_NAN(rad) );
	FastSinCos( rad, &zrot[0][1], &zrot[0][0] );


	IW_ASSERT( !IS_NAN(zrot[0][1]) );
	IW_ASSERT( !IS_NAN(zrot[0][0]) );
	zrot[1][0] = -zrot[0][1];
	zrot[1][1] = zrot[0][0];

	MatrixMultiply( m, zrot, tmpmat );
	MatrixMultiply( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection( vec3_t axis[3], float yaw ) {

	// create an arbitrary axis[1] 
	PerpendicularVector( axis[1], axis[0] );

	// rotate it around axis[0] by yaw
	if ( yaw ) {
		vec3_t	temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	// cross to get axis[2]
	CrossProduct( axis[0], axis[1], axis[2] );
}

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up) {
	float		d;

	// this rotate and negate guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}

/*
=============
vectoyaw
=============
*/
float vectoyaw( const vec3_t vec ) {
	float yaw;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		yaw = 0.0f;
	} else {
		yaw = (float)( 180.0 * atan2( vec[1], vec[0] ) / M_PI );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}
	}
	return yaw;
}

/*
=============
vectosignedyaw

Yaw is measured in degrees in the XY plane, positive toward +Y.
The unsigned helper wraps a negative atan2 result into [0, 360).
This helper preserves the signed range [-180, 180].
A vector with no XY component has zero yaw in either form.
The direction need not have unit length.
The signed pitch and angle helpers below use the same convention.
=============
*/
float vectosignedyaw( const vec3_t vec ) {
	float yaw;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		yaw = 0.0f;
	} else {
		yaw = (float)( 180.0 * atan2( vec[1], vec[0] ) / M_PI );




		IW_ASSERT( yaw >= -180 );
		IW_ASSERT( yaw <= 180 );
	}
	return yaw;
}

/*
=============
vectopitch

Return pitch in degrees, with positive Z corresponding to negative pitch.
The unsigned result wraps into [0, 360).
The vertical cases avoid an indeterminate horizontal direction.
=============
*/
float vectopitch( const vec3_t vec ) {
	float forward, pitch;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		if ( vec[2] > 0.0f ) {
			pitch = 270.0f;
		} else {
			pitch = 90.0f;
		}
	} else {
		forward = (float)sqrt( vec[0] * vec[0] + vec[1] * vec[1] );
		pitch = (float)( -180.0 * atan2( vec[2], forward ) / M_PI );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}
	return pitch;
}

/*
=============
vectosignedpitch

Return the signed pitch of a direction in degrees.
Its horizontal magnitude comes from the XY projection.
Vertical directions produce -90 or +90 degrees.
=============
*/
float vectosignedpitch( const vec3_t vec ) {
	float forward, pitch;

	if ( vec[1] == 0.0f && vec[0] == 0.0f ) {
		if ( vec[2] > 0.0f ) {
			pitch = -90.0f;
		} else {
			pitch = 90.0f;
		}
	} else {
		forward = (float)sqrt( vec[0] * vec[0] + vec[1] * vec[1] );
		pitch = (float)( -180.0 * atan2( vec[2], forward ) / M_PI );
	}
	return pitch;
}

/*
=============
vectoangles

Convert a direction into unsigned pitch and yaw; roll is zero.
No normalization is needed before atan2.
A vertical vector has no defined yaw, so that component is zero.
Negative angular results are wrapped by one full turn.
=============
*/
void vectoangles( const vec3_t value, vec3_t angles ) {
	float forward, yaw, pitch;

	if ( value[1] == 0.0f && value[0] == 0.0f ) {
		yaw = 0.0f;
		if ( value[2] > 0.0f ) {
			pitch = 270.0f;
		} else {
			pitch = 90.0f;
		}
	} else {
		yaw = (float)( 180.0 * atan2( value[1], value[0] ) / M_PI );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}
		forward = (float)sqrt( value[0] * value[0] + value[1] * value[1] );
		pitch = (float)( -180.0 * atan2( value[2], forward ) / M_PI );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.0f;
}

/*
=============
vectosignedangles

Convert a direction into signed pitch and yaw; roll is zero.
Pitch uses the negative elevation of the direction.
Yaw uses atan2 of its XY components without wrapping.
The vertical case selects a pitch and leaves yaw at zero.
=============
*/
void vectosignedangles( const vec3_t value, vec3_t angles ) {
	float forward, yaw, pitch;

	if ( value[1] == 0.0f && value[0] == 0.0f ) {
		yaw = 0.0f;
		if ( value[2] > 0.0f ) {
			pitch = -90.0f;
		} else {
			pitch = 90.0f;
		}
	} else {
		yaw = (float)( 180.0 * atan2( value[1], value[0] ) / M_PI );
		forward = (float)sqrt( value[0] * value[0] + value[1] * value[1] );
		pitch = (float)( -180.0 * atan2( value[2], forward ) / M_PI );
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0.0f;
}

/*
=============
AngleVectors

Construct the forward/right/up vectors for Euler angles in degrees.
Pitch and yaw determine forward; roll only affects right and up.
A null output pointer suppresses that vector.
The right vector points opposite the second axis used by AnglesToAxis.
Compute the roll sine/cosine only when right or up is requested.
=============
*/
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up ) {
	float angle;
	float sr, sp, sy, cr, cp, cy;

	angle = angles[1] * ( M_PI * 2 / 360 );
	FastSinCos( angle, &sy, &cy );

	angle = angles[0] * ( M_PI * 2 / 360 );
	FastSinCos( angle, &sp, &cp );

	if ( forward ) {
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if ( right || up ) {
	angle = angles[2] * ( M_PI * 2 / 360 );
	FastSinCos( angle, &sr, &cr );

	if ( right ) {
		right[0] = ( -1 * sr * sp * cy + -1 * cr * -sy );
		right[1] = ( -1 * sr * sp * sy + -1 * cr * cy );
		right[2] = -1 * sr * cp;
	}

	if ( up ) {
		up[0] = ( cr * sp * cy + -sr * -sy );
		up[1] = ( cr * sp * sy + -sr * cy );
		up[2] = cr * cp;
	}
	}
}

/*
=============
YawVectors

Construct horizontal forward and right vectors from a yaw in degrees.
Both outputs are optional and have a zero Z component.
This avoids computing pitch/roll for planar directions.
=============
*/
void YawVectors( float yaw, vec3_t forward, vec3_t right ) {
	float angle, sy, cy;
	angle = yaw * ( M_PI * 2 / 360 );
	FastSinCos( angle, &sy, &cy );
	if ( forward ) {
		forward[0] = cy;
		forward[1] = sy;
		forward[2] = 0;
	}
	if ( right ) {
		right[0] = sy;
		right[1] = -cy;
		right[2] = 0;
	}
}

/*
=============
PerpendicularVector

Choose the cardinal axis least aligned with the source.
Project that axis into the plane perpendicular to the source.
Normalize the result to obtain a stable unit perpendicular.
The source must be nonzero.
=============
*/
void PerpendicularVector( vec3_t dst, const vec3_t src )
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	IW_ASSERT( src[0] || src[1] || src[2] );

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem )
		{
			minelem = fabs( src[i] );
			pos = i;
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

/*
GetPerpendicularViewVector
*/
void GetPerpendicularViewVector( const vec3_t a, const vec3_t origin,
						 const vec3_t b, vec3_t normal ) {
	vec3_t v1, v2;

	VectorSubtract( a, origin, v1 );
	VectorNormalize( v1 );
	VectorSubtract( a, b, v2 );
	VectorNormalize( v2 );
	CrossProduct( v1, v2, normal );
	VectorNormalize( normal );
}

/*
ProjectPointOntoVector
*/
void ProjectPointOntoVector( const vec3_t point, const vec3_t origin,
							 const vec3_t linePoint, vec3_t out ) {
	vec3_t pVec, vec;

	VectorSubtract( point, origin, pVec );
	VectorSubtract( linePoint, origin, vec );
	VectorNormalize( vec );
	// project onto the directional vector for this segment
	VectorMA( origin, DotProduct( pVec, vec ), vec, out );
}

/*
Matrix storage conventions:
The 3x3 routines operate on the linear part of a transform.
A 3x4 affine matrix stores its translation in the fourth column.
A 4x3 affine matrix stores its translation in the fourth row.
Composition includes that translation only in the affine row/column.
The Equals variant saves rows before overwriting its input.
The ordinary multiply routines require a separate output matrix.
Transposition exchanges row and column indices; it is not a general inverse.
*/
/*
MatrixMultiply
*/
void MatrixMultiply( const float in1[3][3], const float in2[3][3], float out[3][3] ) {
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2];
}

/*
MatrixMultiplyEquals
*/
void MatrixMultiplyEquals( const float left[3][3], float inout[3][3] ) {
	vec3_t temp[2];

	/* Save the first two result rows before overwriting the input. */
	temp[0][0] = left[0][0] * inout[0][0] + left[0][1] * inout[1][0] + left[0][2] * inout[2][0];
	temp[0][1] = left[0][0] * inout[0][1] + left[0][1] * inout[1][1] + left[0][2] * inout[2][1];
	temp[0][2] = left[0][0] * inout[0][2] + left[0][1] * inout[1][2] + left[0][2] * inout[2][2];
	temp[1][0] = left[1][0] * inout[0][0] + left[1][1] * inout[1][0] + left[1][2] * inout[2][0];
	temp[1][1] = left[1][0] * inout[0][1] + left[1][1] * inout[1][1] + left[1][2] * inout[2][1];
	temp[1][2] = left[1][0] * inout[0][2] + left[1][1] * inout[1][2] + left[1][2] * inout[2][2];
	inout[2][0] = left[2][0] * inout[0][0] + left[2][1] * inout[1][0] + left[2][2] * inout[2][0];
	inout[2][1] = left[2][0] * inout[0][1] + left[2][1] * inout[1][1] + left[2][2] * inout[2][1];
	inout[2][2] = left[2][0] * inout[0][2] + left[2][1] * inout[1][2] + left[2][2] * inout[2][2];
	VectorCopy( temp[0], inout[0] );
	VectorCopy( temp[1], inout[1] );
}

/*
MatrixMultiply34
*/
void MatrixMultiply34( const float left[3][4], const float right[3][4], float out[3][4] ) {
	out[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] +
		left[0][2] * right[2][0];
	out[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] +
		left[0][2] * right[2][1];
	out[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] +
		left[0][2] * right[2][2];
	out[0][3] = left[0][0] * right[0][3] + left[0][1] * right[1][3] +
		left[0][2] * right[2][3] + left[0][3];
	out[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] +
		left[1][2] * right[2][0];
	out[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] +
		left[1][2] * right[2][1];
	out[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] +
		left[1][2] * right[2][2];
	out[1][3] = left[1][0] * right[0][3] + left[1][1] * right[1][3] +
		left[1][2] * right[2][3] + left[1][3];
	out[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] +
		left[2][2] * right[2][0];
	out[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] +
		left[2][2] * right[2][1];
	out[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] +
		left[2][2] * right[2][2];
	out[2][3] = left[2][0] * right[0][3] + left[2][1] * right[1][3] +
		left[2][2] * right[2][3] + left[2][3];
}

/*
MatrixMultiply43
*/
void MatrixMultiply43( const float left[4][3], const float right[4][3], float out[4][3] ) {
	out[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] +
		left[0][2] * right[2][0];
	out[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] +
		left[1][2] * right[2][0];
	out[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] +
		left[2][2] * right[2][0];
	out[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] +
		left[0][2] * right[2][1];
	out[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] +
		left[1][2] * right[2][1];
	out[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] +
		left[2][2] * right[2][1];
	out[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] +
		left[0][2] * right[2][2];
	out[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] +
		left[1][2] * right[2][2];
	out[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] +
		left[2][2] * right[2][2];
	out[3][0] = left[3][0] * right[0][0] + left[3][1] * right[1][0] +
		left[3][2] * right[2][0] + right[3][0];
	out[3][1] = left[3][0] * right[0][1] + left[3][1] * right[1][1] +
		left[3][2] * right[2][1] + right[3][1];
	out[3][2] = left[3][0] * right[0][2] + left[3][1] * right[1][2] +
		left[3][2] * right[2][2] + right[3][2];
}

/*
DObjSkelMatrixMultiply43
*/
void DObjSkelMatrixMultiply43( const float left[4][4], const float right[4][3], float out[4][3] ) {
	out[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] +
		left[0][2] * right[2][0];
	out[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] +
		left[1][2] * right[2][0];
	out[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] +
		left[2][2] * right[2][0];
	out[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] +
		left[0][2] * right[2][1];
	out[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] +
		left[1][2] * right[2][1];
	out[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] +
		left[2][2] * right[2][1];
	out[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] +
		left[0][2] * right[2][2];
	out[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] +
		left[1][2] * right[2][2];
	out[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] +
		left[2][2] * right[2][2];
	out[3][0] = left[3][0] * right[0][0] + left[3][1] * right[1][0] +
		left[3][2] * right[2][0] + right[3][0];
	out[3][1] = left[3][0] * right[0][1] + left[3][1] * right[1][1] +
		left[3][2] * right[2][1] + right[3][1];
	out[3][2] = left[3][0] * right[0][2] + left[3][1] * right[1][2] +
		left[3][2] * right[2][2] + right[3][2];
}

/*
DObjSkel2MatrixMultiply43
*/
void DObjSkel2MatrixMultiply43( const float left[4][4], const float right[4][3], float out[4][4] ) {
	out[0][0] = left[0][0] * right[0][0] + left[0][1] * right[1][0] +
		left[0][2] * right[2][0];
	out[1][0] = left[1][0] * right[0][0] + left[1][1] * right[1][0] +
		left[1][2] * right[2][0];
	out[2][0] = left[2][0] * right[0][0] + left[2][1] * right[1][0] +
		left[2][2] * right[2][0];
	out[0][1] = left[0][0] * right[0][1] + left[0][1] * right[1][1] +
		left[0][2] * right[2][1];
	out[1][1] = left[1][0] * right[0][1] + left[1][1] * right[1][1] +
		left[1][2] * right[2][1];
	out[2][1] = left[2][0] * right[0][1] + left[2][1] * right[1][1] +
		left[2][2] * right[2][1];
	out[0][2] = left[0][0] * right[0][2] + left[0][1] * right[1][2] +
		left[0][2] * right[2][2];
	out[1][2] = left[1][0] * right[0][2] + left[1][1] * right[1][2] +
		left[1][2] * right[2][2];
	out[2][2] = left[2][0] * right[0][2] + left[2][1] * right[1][2] +
		left[2][2] * right[2][2];
	out[0][3] = 0.0f;
	out[1][3] = 0.0f;
	out[2][3] = 0.0f;
	out[3][0] = left[3][0] * right[0][0] + left[3][1] * right[1][0] +
		left[3][2] * right[2][0] + right[3][0];
	out[3][1] = left[3][0] * right[0][1] + left[3][1] * right[1][1] +
		left[3][2] * right[2][1] + right[3][1];
	out[3][2] = left[3][0] * right[0][2] + left[3][1] * right[1][2] +
		left[3][2] * right[2][2] + right[3][2];
	out[3][3] = 1.0f;
}

/*
MatrixTranspose
*/
void MatrixTranspose( const float in[3][3], float out[3][3] ) {
	out[0][0] = in[0][0];
	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[1][0] = in[0][1];
	out[1][1] = in[1][1];
	out[1][2] = in[2][1];
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
	out[2][2] = in[2][2];
}

/*
MatrixInverse
*/
void MatrixInverse( const float in[9], float out[9] ) {
	float det;

	det = ( in[8] * in[4] - in[7] * in[5] ) * in[0]
		- ( in[8] * in[1] - in[7] * in[2] ) * in[3]
		+ ( in[5] * in[1] - in[4] * in[2] ) * in[6];

	IW_ASSERT( det );
	det = 1.0f / det;

	out[0] =  ( in[8] * in[4] - in[7] * in[5] ) * det;
	out[1] = -( in[8] * in[1] - in[7] * in[2] ) * det;
	out[2] =  ( in[5] * in[1] - in[4] * in[2] ) * det;
	out[3] = -( in[8] * in[3] - in[6] * in[5] ) * det;
	out[4] =  ( in[8] * in[0] - in[6] * in[2] ) * det;
	out[5] = -( in[5] * in[0] - in[3] * in[2] ) * det;
	out[6] =  ( in[7] * in[3] - in[6] * in[4] ) * det;
	out[7] = -( in[7] * in[0] - in[6] * in[1] ) * det;
	out[8] =  ( in[4] * in[0] - in[3] * in[1] ) * det;
}



/*
MatrixInverseOrthogonal43
*/
void MatrixInverseOrthogonal43( const float in[4][3], float out[4][3] ) {
	vec3_t origin;

	MatrixTranspose( in, out );
	VectorSubtract( vec3_origin, in[3], origin );
	MatrixTransformVector( origin, out, out[3] );
}

/*
MatrixInverse44

Form the adjugate, then divide it by the determinant.
Transpose the input into src so the cofactor indexing stays regular.
This also keeps the original values available for an in-place inverse.
Cache twelve pair products from the final two source rows.
Use those products for the first eight cofactor entries.
Replace the cache with products from the first two rows for the rest.
Each cofactor is a positive three-term sum minus a negative three-term sum.
The determinant is the dot product of a source row and its cofactors.
The caller must supply a nonsingular matrix.
*/
void MatrixInverse44( const float in[16], float out[16] ) {
	float tmp[12];
	float src[16];
	float determinant;
	int i;

	for ( i = 0; i < 4; ++i ) {
		src[i] = in[i * 4];
		src[i + 4] = in[i * 4 + 1];
		src[i + 8] = in[i * 4 + 2];
		src[i + 12] = in[i * 4 + 3];
	}

	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];

	out[0] = tmp[0] * src[5] +
		tmp[3] * src[6] +
		tmp[4] * src[7];
	out[0] -= tmp[1] * src[5] +
		tmp[2] * src[6] +
		tmp[5] * src[7];
	out[1] = tmp[1] * src[4] +
		tmp[6] * src[6] +
		tmp[9] * src[7];
	out[1] -= tmp[0] * src[4] +
		tmp[7] * src[6] +
		tmp[8] * src[7];
	out[2] = tmp[2] * src[4] +
		tmp[7] * src[5] +
		tmp[10] * src[7];
	out[2] -= tmp[3] * src[4] +
		tmp[6] * src[5] +
		tmp[11] * src[7];
	out[3] = tmp[5] * src[4] +
		tmp[8] * src[5] +
		tmp[11] * src[6];
	out[3] -= tmp[4] * src[4] +
		tmp[9] * src[5] +
		tmp[10] * src[6];
	out[4] = tmp[1] * src[1] +
		tmp[2] * src[2] +
		tmp[5] * src[3];
	out[4] -= tmp[0] * src[1] +
		tmp[3] * src[2] +
		tmp[4] * src[3];
	out[5] = tmp[0] * src[0] +
		tmp[7] * src[2] +
		tmp[8] * src[3];
	out[5] -= tmp[1] * src[0] +
		tmp[6] * src[2] +
		tmp[9] * src[3];
	out[6] = tmp[3] * src[0] +
		tmp[6] * src[1] +
		tmp[11] * src[3];
	out[6] -= tmp[2] * src[0] +
		tmp[7] * src[1] +
		tmp[10] * src[3];
	out[7] = tmp[4] * src[0] +
		tmp[9] * src[1] +
		tmp[10] * src[2];
	out[7] -= tmp[5] * src[0] +
		tmp[8] * src[1] +
		tmp[11] * src[2];

	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];

	out[8] = tmp[0] * src[13] +
		tmp[3] * src[14] +
		tmp[4] * src[15];
	out[8] -= tmp[1] * src[13] +
		tmp[2] * src[14] +
		tmp[5] * src[15];
	out[9] = tmp[1] * src[12] +
		tmp[6] * src[14] +
		tmp[9] * src[15];
	out[9] -= tmp[0] * src[12] +
		tmp[7] * src[14] +
		tmp[8] * src[15];
	out[10] = tmp[2] * src[12] +
		tmp[7] * src[13] +
		tmp[10] * src[15];
	out[10] -= tmp[3] * src[12] +
		tmp[6] * src[13] +
		tmp[11] * src[15];
	out[11] = tmp[5] * src[12] +
		tmp[8] * src[13] +
		tmp[11] * src[14];
	out[11] -= tmp[4] * src[12] +
		tmp[9] * src[13] +
		tmp[10] * src[14];
	out[12] = tmp[2] * src[10] +
		tmp[5] * src[11] +
		tmp[1] * src[9];
	out[12] -= tmp[4] * src[11] +
		tmp[0] * src[9] +
		tmp[3] * src[10];
	out[13] = tmp[8] * src[11] +
		tmp[0] * src[8] +
		tmp[7] * src[10];
	out[13] -= tmp[6] * src[10] +
		tmp[9] * src[11] +
		tmp[1] * src[8];
	out[14] = tmp[6] * src[9] +
		tmp[11] * src[11] +
		tmp[3] * src[8];
	out[14] -= tmp[10] * src[11] +
		tmp[2] * src[8] +
		tmp[7] * src[9];
	out[15] = tmp[10] * src[10] +
		tmp[4] * src[8] +
		tmp[9] * src[9];
	out[15] -= tmp[8] * src[9] +
		tmp[11] * src[10] +
		tmp[5] * src[8];

	determinant = src[0] * out[0] + src[1] * out[1]
		+ src[2] * out[2] + src[3] * out[3];
	determinant = 1.0f / determinant;
	for ( i = 0; i < 16; ++i ) {
		out[i] *= determinant;
	}
}

/*
MatrixTransformVector
*/
__inline void MatrixTransformVector( vec3_t v, float r[3][3], vec3_t d ) {
	d[0] = v[0] * r[0][0] + v[1] * r[1][0] + v[2] * r[2][0];
	d[1] = v[0] * r[0][1] + v[1] * r[1][1] + v[2] * r[2][1];
	d[2] = v[0] * r[0][2] + v[1] * r[1][2] + v[2] * r[2][2];
}

/*
MatrixTransposeTransformVector
*/
void MatrixTransposeTransformVector( const vec3_t in, const float m[3][3], vec3_t out ) {
	out[0] = in[0] * m[0][0] + in[1] * m[0][1] + in[2] * m[0][2];
	out[1] = in[0] * m[1][0] + in[1] * m[1][1] + in[2] * m[1][2];
	out[2] = in[0] * m[2][0] + in[1] * m[2][1] + in[2] * m[2][2];
}

/*
MatrixTransformVector43
*/
void MatrixTransformVector43( const float *in, const float m[4][3], float *out ) {
	out[0] = in[0] * m[0][0] + in[1] * m[1][0] + in[2] * m[2][0] + m[3][0];
	out[1] = in[0] * m[0][1] + in[1] * m[1][1] + in[2] * m[2][1] + m[3][1];
	out[2] = in[0] * m[0][2] + in[1] * m[1][2] + in[2] * m[2][2] + m[3][2];
}

/*
DObjSkelMatrixTransformVector43
*/
void DObjSkelMatrixTransformVector43( const vec3_t in, const float m[4][4], vec3_t out ) {
	out[0] = in[0] * m[0][0] + in[1] * m[1][0] + in[2] * m[2][0] + m[3][0];
	out[1] = in[0] * m[0][1] + in[1] * m[1][1] + in[2] * m[2][1] + m[3][1];
	out[2] = in[0] * m[0][2] + in[1] * m[1][2] + in[2] * m[2][2] + m[3][2];
}

/*
MatrixTransposeTransformVector43
*/
void MatrixTransposeTransformVector43( const vec3_t in, const float m[4][3], vec3_t out ) {
	vec3_t temp;

	VectorSubtract( in, m[3], temp );
	out[0] = DotProduct( temp, m[0] );
	out[1] = DotProduct( temp, m[1] );
	out[2] = DotProduct( temp, m[2] );
}

/*
MatrixTransformVector43Equals
*/
void MatrixTransformVector43Equals( vec3_t v, const float matrix[4][3] ) {
	vec2_t temp;

	/* Keep the original components until all three products are complete. */
	temp[0] = v[0] * matrix[0][0] + v[1] * matrix[1][0] +
		v[2] * matrix[2][0] + matrix[3][0];
	temp[1] = v[0] * matrix[0][1] + v[1] * matrix[1][1] +
		v[2] * matrix[2][1] + matrix[3][1];
	v[2] = v[0] * matrix[0][2] + v[1] * matrix[1][2] +
		v[2] * matrix[2][2] + matrix[3][2];
	v[0] = temp[0];
	v[1] = temp[1];
}

/*
VectorAngleMultiply
*/
void VectorAngleMultiply( vec3_t v, float degrees ) {
	float x, y, temp;
	FastSinCos( degrees * ( M_PI * 2 / 360 ), &y, &x );
	temp = x * v[0] - y * v[1];
	v[1] = x * v[1] + y * v[0];
	v[0] = temp;
}

/*
Quaternion helpers use component order (x, y, z, w).
Multiplication composes rotations; inversion negates the vector part.
A unit quaternion and its negative represent the same rotation.
The matrix conversion compensates for the quaternion magnitude.
EigenTrace measures the vector-part magnitude relative to the full norm.
For a relative rotation it supplies a sign-independent angular error.
*/
/*
QuatMultiply
*/
void QuatMultiply( const vec4_t a, const vec4_t b, vec4_t out ) {
	out[0] = a[0] * b[3] + a[3] * b[0] + a[2] * b[1] - a[1] * b[2];
	out[1] = a[1] * b[3] - a[2] * b[0] + a[3] * b[1] + a[0] * b[2];
	out[2] = a[2] * b[3] + a[1] * b[0] - a[0] * b[1] + a[3] * b[2];
	out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}

/*
QuatInverse
*/
void QuatInverse( const vec4_t in, vec4_t out ) {
	out[0] = -in[0];
	out[1] = -in[1];
	out[2] = -in[2];
	out[3] = in[3];
}

/*
ConvertQuatToMat
*/
void ConvertQuatToMat( float quatAndMatrix[9] ) {
	float xx, xy, xz, xw, yy, yz, yw, zz, zw, r;

	xx = quatAndMatrix[0] * quatAndMatrix[0];
	yy = quatAndMatrix[1] * quatAndMatrix[1];
	zz = quatAndMatrix[2] * quatAndMatrix[2];
	r = xx + yy + zz + quatAndMatrix[3] * quatAndMatrix[3];
	if ( r != 0.0f ) {
		r = 2.0f / r;
		xx *= r;
		yy *= r;
		zz *= r;
		quatAndMatrix[0] *= r;
		xy = quatAndMatrix[0] * quatAndMatrix[1];
		xz = quatAndMatrix[0] * quatAndMatrix[2];
		xw = quatAndMatrix[0] * quatAndMatrix[3];
		quatAndMatrix[1] *= r;
		yz = quatAndMatrix[1] * quatAndMatrix[2];
		yw = quatAndMatrix[1] * quatAndMatrix[3];
		zw = quatAndMatrix[2] * quatAndMatrix[3] * r;
		quatAndMatrix[0] = 1.0f - ( yy + zz );
		quatAndMatrix[1] = xy + zw;
		quatAndMatrix[2] = xz - yw;
		quatAndMatrix[3] = xy - zw;
		quatAndMatrix[4] = 1.0f - ( xx + zz );
		quatAndMatrix[5] = yz + xw;
		quatAndMatrix[6] = xz + yw;
		quatAndMatrix[7] = yz - xw;
		quatAndMatrix[8] = 1.0f - ( xx + yy );
	} else {
		quatAndMatrix[0] = 1.0f;
		quatAndMatrix[1] = 0.0f;
		quatAndMatrix[2] = 0.0f;
		quatAndMatrix[3] = 0.0f;
		quatAndMatrix[4] = 1.0f;
		quatAndMatrix[5] = 0.0f;
		quatAndMatrix[6] = 0.0f;
		quatAndMatrix[7] = 0.0f;
		quatAndMatrix[8] = 1.0f;
	}
}

/*
QuatEigenTrace
*/
float QuatEigenTrace( const vec4_t q ) {
	float xx, yy, zz, lengthSquared;
	xx = q[0] * q[0];
	yy = q[1] * q[1];
	zz = q[2] * q[2];
	lengthSquared = xx + yy + zz + q[3] * q[3];
	if ( lengthSquared != 0.0f ) {
		lengthSquared = 1.0f / lengthSquared;
		xx *= lengthSquared;
		yy *= lengthSquared;
		zz *= lengthSquared;
		return xx + yy + zz;
	}
	return 0.0f;
}

/*
AngleEigenTrace
*/
float AngleEigenTrace( float degrees ) {
	double s = sin( degrees * ( 3.14159265358979323846f / 180.0f ) );
	return (float)( s * s );
}

/*
QuatRatioEigenTrace
*/
float QuatRatioEigenTrace( const vec4_t a, const vec4_t b ) {
	vec4_t invQuat2, ratioQuat;
	QuatInverse( b, invQuat2 );
	QuatMultiply( a, invQuat2, ratioQuat );
	return QuatEigenTrace( ratioQuat );
}

/*
RotationToYaw
*/
float RotationToYaw( const float v[2] ) {
	float xx, r;

	xx = v[0] * v[0];

	r = xx + v[1] * v[1];
	IW_ASSERT( r );
	r = 2.0f / r;

	return (float)( atan2( v[0] * v[1] * r, 1.0f - xx * r ) *
		( 180.0f / M_PI ) );
}

/*
=============
PitchToQuaternion

Build a quaternion for a pure pitch rotation in degrees.
The Y component and scalar component contain the half-angle sine/cosine.
=============
*/
void PitchToQuaternion( float degrees, vec4_t q ) {
	degrees = degrees * ( M_PI / 360.0f );
	q[0] = 0.0f;
	q[2] = 0.0f;
	FastSinCos( degrees, &q[1], &q[3] );
}

/*
=============
YawToQuaternion

Build a quaternion for a pure yaw rotation in degrees.
Only the Z and scalar components can be nonzero.
=============
*/
void YawToQuaternion( float degrees, vec4_t q ) {
	degrees = degrees * ( M_PI / 360.0f );
	q[0] = 0.0f;
	q[1] = 0.0f;
	FastSinCos( degrees, &q[2], &q[3] );
}

/*
=============
RollToQuaternion

Build a quaternion for a pure roll rotation in degrees.
Only the X and scalar components can be nonzero.
=============
*/
void RollToQuaternion( float degrees, vec4_t q ) {
	degrees = degrees * ( M_PI / 360.0f );
	q[1] = 0.0f;
	q[2] = 0.0f;
	FastSinCos( degrees, &q[0], &q[3] );
}

/*
=============
ColorBytes3

Pack RGB components scaled by 255 into the low three bytes.
The unused fourth byte is left zero.
=============
*/
unsigned ColorBytes3( float r, float g, float b ) {
	unsigned i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = 0xff;

	return i;
}

/*
=============
ColorBytes4

Pack RGBA components scaled by 255 into a machine integer.
Byte order follows the little-endian converter file convention.
=============
*/
unsigned ColorBytes4 (float r, float g, float b, float a) {
	unsigned	i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

/*
=============
NormalizeColor

Scale RGB by the largest component and return that component.
A zero maximum produces a zero output vector.
=============
*/
float NormalizeColor( const vec3_t in, vec3_t out ) {
	float	max;
	
	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}

/*
=============
AngleMod

Quantize a degree angle to a 16-bit turn and wrap it.
The mask supplies modulo behavior without repeated subtraction.
=============
*/
float AngleMod( float a ) {
	return ( 360.0f / 65536 ) * ( (int)( a * ( 65536 / 360.0f ) ) & 65535 );
}

/*
===============
LerpAngle

Adjust the destination by one turn to choose the nearer arc.
Interpolate on that arc without normalizing the final result.

===============
*/
float LerpAngle( float from, float to, float frac ) {
	float result;
	float a;

	a = to;
	if ( a - from > 180 ) {
		a -= 360;
	}
	if ( a - from < -180 ) {
		a += 360;
	}
	result = ( a - from ) * frac + from;
	return result;
}

/*
=================
AngleSubtract

Repeatedly remove full turns from the difference.
The signs of exact half-turn endpoints are preserved.

Always returns a value from -180 to 180
=================
*/
float	AngleSubtract( float a1, float a2 ) {
	float	a;

	a = a1 - a2;
	while ( a > 180 ) {
		a -= 360;
	}
	while ( a < -180 ) {
		a += 360;
	}
	return a;
}

/*
=============
AnglesSubtract

Compute an independently wrapped difference on each Euler axis.
Inputs and output use the same pitch/yaw/roll component order.
=============
*/
void AnglesSubtract( const vec3_t v1, const vec3_t v2, vec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}

/*
=================
AngleNormalize360

The implementation quantizes through a 16-bit turn.
Use the Accurate variant when that quantization is undesirable.

returns angle normalized to the range [0 <= angle < 360]
=================
*/
float AngleNormalize360( float angle ) {
	return ( 360.0f / 65536 ) * ( (int)( angle * ( 65536 / 360.0f ) ) & 65535 );
}

/*
=================
AngleNormalize180

First wrap the angle through the 16-bit unsigned representation.
Then move the upper half of the turn into the negative range.

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
float AngleNormalize180( float angle ) {
	float normalized;

	normalized = AngleNormalize360( angle );
	if ( normalized > 180.0f ) {
		normalized -= 360.0f;
	}
	return normalized;
}

/*
=============
AngleNormalize360Accurate

Wrap by adding/subtracting full turns without fixed-point conversion.
Inputs already in range pass through without changing their fraction.
=============
*/
float AngleNormalize360Accurate( float angle ) {
	if ( angle < 0.0f ) {
		do {
			angle += 360.0f;
		} while ( angle < 0.0f );
		return angle;
	}
	if ( angle >= 360.0f ) {
		do {
			angle -= 360.0f;
		} while ( angle >= 360.0f );
		return angle;
	}
	return angle;
}

/*
=============
AngleNormalize180Accurate

Wrap directly into the signed half-turn interval.
The negative half-turn endpoint wraps to the positive endpoint.
=============
*/
float AngleNormalize180Accurate( float angle ) {
	if ( angle <= -180.0f ) {
		do {
			angle += 360.0f;
		} while ( angle <= -180.0f );
		return angle;
	}
	if ( angle > 180.0f ) {
		do {
			angle -= 360.0f;
		} while ( angle > 180.0f );
		return angle;
	}
	return angle;
}

/*
=================
AngleDelta

Subtract two degree angles using the signed wrapping convention.
This helper retains the fixed-point normalization of AngleNormalize180.

returns the normalized delta from angle1 to angle2
=================
*/
float AngleDelta ( float angle1, float angle2 ) {
	return AngleNormalize180( angle1 - angle2 );
}

/*
=================
RadiusFromBounds

Choose the farthest absolute bound on each axis.
The length of that corner bounds the box about the origin.
=================
*/
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	int i;
	vec3_t corner;
	float a, b;

	for ( i = 0; i < 3; i++ ) {
		a = fabs( mins[i] );
		b = fabs( maxs[i] );
		corner[i] = a > b ? a : b;
	}
	return sqrt( DotProduct( corner, corner ) );
}

/*
=============
ClearBounds

Initialize minima high and maxima low before accumulating points.
This sentinel box is deliberately empty.
=============
*/
void ClearBounds (vec3_t mins, vec3_t maxs)
{
	mins[0] = mins[1] = mins[2] = MAX_BOUNDS_COORD;
	maxs[0] = maxs[1] = maxs[2] = -MAX_BOUNDS_COORD;
}

/*
=============
AddPointToBounds

Expand an axis-aligned box to include a single point.
Each component updates its minimum and maximum independently.
=============
*/
void AddPointToBounds( const float *v, float *mins, float *maxs ) {
	if ( v[0] < mins[0] ) mins[0] = v[0];
	if ( v[0] > maxs[0] ) maxs[0] = v[0];
	if ( v[1] < mins[1] ) mins[1] = v[1];
	if ( v[1] > maxs[1] ) maxs[1] = v[1];
	if ( v[2] < mins[2] ) mins[2] = v[2];
	if ( v[2] > maxs[2] ) maxs[2] = v[2];
}

/*
=============
ExpandBounds

Grow existing bounds to contain another complete box.
The input bounds and output bounds use the same coordinate space.
=============
*/
void ExpandBounds( const vec3_t mins, const vec3_t maxs, vec3_t outMins, vec3_t outMaxs ) {
	if ( outMins[0] > mins[0] ) outMins[0] = mins[0];
	if ( outMaxs[0] < maxs[0] ) outMaxs[0] = maxs[0];
	if ( outMins[1] > mins[1] ) outMins[1] = mins[1];
	if ( outMaxs[1] < maxs[1] ) outMaxs[1] = maxs[1];
	if ( outMins[2] > mins[2] ) outMins[2] = mins[2];
	if ( outMaxs[2] < maxs[2] ) outMaxs[2] = maxs[2];
}

/*
=============
AxisClear

Set the three orientation axes to the identity basis.
Each row is a unit direction and all off-diagonal entries are zero.
=============
*/
void AxisClear( vec3_t axis[3] ) {
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

/*
=============
AxisCopy

Copy all three orientation axes without changing their convention.
VectorCopy handles each corresponding row.
=============
*/
void AxisCopy( vec3_t in[3], vec3_t out[3] ) {
	VectorCopy( in[0], out[0] );
	VectorCopy( in[1], out[1] );
	VectorCopy( in[2], out[2] );
}

/*
=================
AnglesToAxis

Build an orientation basis from pitch/yaw/roll in degrees.
Negate AngleVectors right to obtain the stored second axis.
=================
*/
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] ) {
	vec3_t	right;

	// angle vectors returns "right" instead of "y axis"
	AngleVectors( angles, axis[0], right, axis[2] );
	VectorSubtract( vec3_origin, right, axis[1] );
}

/*
=============
YawToAxis

Construct an orientation whose only rotation is yaw.
The forward/second axes stay horizontal and the third axis remains up.
=============
*/
void YawToAxis( float yaw, vec3_t axis[3] ) {
	vec3_t right;

	YawVectors( yaw, axis[0], right );
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
	VectorSubtract( vec3_origin, right, axis[1] );
}

/*
=================
AxisToAngles

Read pitch/yaw from the forward axis before recovering roll.
Undo yaw and pitch on the second axis to isolate that final rotation.
Its orientation selects the correct roll branch.
The resulting unsigned angles follow the vector-to-angle helpers.

  Used to convert the MD3 tag axis to MDC tag angles, which are much smaller

  This doesn't have to be fast, since it's only used for conversion in utils, try to avoid
  using this during gameplay
=================
*/
void AxisToAngles( const vec3_t axis[3], vec3_t angles ) {
	vec3_t right;
	float angle, fSin, fCos, t, roll;

	// first get the pitch and yaw from the forward vector
	vectoangles( axis[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( axis[1], right );

	angle = ( -angles[1] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	t        = fCos * right[0] - fSin * right[1];
	right[1] = fSin * right[0] + fCos * right[1];

	angle = ( -angles[0] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	right[0] = fSin * right[2] + fCos * t;
	right[2] = fCos * right[2] - fSin * t;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = roll + ( roll < 0.0f ? 180.0f : -180.0f );
	} else {
		angles[2] = -roll;
	}
}

/*
=============
Axis4ToAngles

Read Euler angles from the linear portion of a 4-row transform.
The translation row has no effect on the orientation.
Reverse yaw and pitch on the second axis to recover roll.
Use the same unsigned convention as AxisToAngles.
=============
*/
void Axis4ToAngles( const float matrix[3][4], vec3_t angles ) {
	vec3_t right;
	float angle, fSin, fCos, t, roll;

	// first get the pitch and yaw from the forward vector
	vectoangles( matrix[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( matrix[1], right );

	angle = ( -angles[1] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	t        = fCos * right[0] - fSin * right[1];
	right[1] = fSin * right[0] + fCos * right[1];

	angle = ( -angles[0] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	right[0] = fSin * right[2] + fCos * t;
	right[2] = fCos * right[2] - fSin * t;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = roll + ( roll < 0.0f ? 180.0f : -180.0f );
	} else {
		angles[2] = -roll;
	}
}



/*
=============
AxisToSignedAngles

Read signed pitch/yaw from the forward axis.
Reverse those rotations on the second axis before recovering roll.
The sign of its remaining side component selects the half-turn branch.
Keep the signed vector-angle convention throughout.
=============
*/
void AxisToSignedAngles( const vec3_t axis[3], vec3_t angles ) {
	vec3_t right;
	float angle, fSin, fCos, t, roll;

	// first get the pitch and yaw from the forward vector
	vectosignedangles( axis[0], angles );

	// now get the roll from the right vector, reverse-rotated by yaw then pitch
	VectorCopy( axis[1], right );

	angle = ( -angles[1] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	t        = fCos * right[0] - fSin * right[1];
	right[1] = fSin * right[0] + fCos * right[1];

	angle = ( -angles[0] ) * M_PI / 180.0f;
	FastSinCos( angle, &fSin, &fCos );
	right[0] = fSin * right[2] + fCos * t;
	right[2] = fCos * right[2] - fSin * t;

	roll = vectosignedpitch( right );
	if ( right[1] < 0.0f ) {
		angles[2] = roll + ( roll < 0.0f ? 180.0f : -180.0f );
	} else {
		angles[2] = -roll;
	}
}

/*
=====================
PlaneFromPoints

Build a plane from the winding order of three points.
The cross product supplies the normal, then normalization supplies length.
A zero-length normal identifies a degenerate triangle.
The plane distance is the normalized normal dotted with the first point.

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints( float *plane, const float *a, const float *b, const float *c ) {
	float d1[3], d2[3];
	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, plane );
	if ( VectorNormalize( plane ) == 0 ) return 0;
	plane[3] = DotProduct( a, plane );
	return 1;
}

/*
=============
ProjectPointOnPlane

Remove the normal component of a point for a normalized plane normal.
The plane passes through the coordinate origin.
Checks reject a zero squared normal length and an invalid reciprocal.
Both the scalar projection and copied normal use that reciprocal.
=============
*/
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
	float d;
	vec3_t n;
	float inv_denom, lengthSqrd;

	lengthSqrd = DotProduct( normal, normal );


	IW_ASSERT( lengthSqrd );
	inv_denom = 1.0F / lengthSqrd;


	IW_ASSERT( !IS_NAN(inv_denom) );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
=================
SetPlaneSignbits

Encode each negative normal component as its corresponding octant bit.
=================
*/
void SetPlaneSignbits( cplane_t *out ) {
	byte bits;
	int j;

	bits = 0;
	for ( j = 0; j < 3; j++ ) {
		if ( out->normal[j] < 0 ) {
			bits |= 1 << j;
		}
	}
	out->signbits = bits;
}

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2

// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}

==================
*/

#if !( (defined __linux__ || __FreeBSD__) && (defined __i386__) && (!defined C_ONLY)) // rb010123

#if defined __LCC__ || defined C_ONLY || !defined(_M_IX86) || defined __VECTORC

int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float	dist1, dist2;
	int		sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}
#else
#pragma warning( disable: 4035 )

/*
=============
BoxOnPlaneSide

The normal octant selects the near/far support corners of the box.
Comparing both support distances sets the front/back result bits.
The x86 path rejects sign bits outside the eight valid octants.
=============
*/
__declspec( naked ) int BoxOnPlaneSide( vec3_t emins, vec3_t emaxs, struct cplane_s *p )
{
	static int bops_initialized;
	static int Ljmptab[8];

	__asm {

		push ebx
			
		cmp bops_initialized, 1
		je  initialized
		mov bops_initialized, 1
		
		mov Ljmptab[0*4], offset Lcase0
		mov Ljmptab[1*4], offset Lcase1
		mov Ljmptab[2*4], offset Lcase2
		mov Ljmptab[3*4], offset Lcase3
		mov Ljmptab[4*4], offset Lcase4
		mov Ljmptab[5*4], offset Lcase5
		mov Ljmptab[6*4], offset Lcase6
		mov Ljmptab[7*4], offset Lcase7
			
initialized:

		mov edx,dword ptr[4+12+esp]
		mov ecx,dword ptr[4+4+esp]
		xor eax,eax
		mov ebx,dword ptr[4+8+esp]
		mov al,byte ptr[17+edx]
		cmp al,8
		jae Lerror
		fld dword ptr[0+edx]
		fld st(0)
		jmp dword ptr[Ljmptab+eax*4]
Lcase0:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase1:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase2:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase3:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase4:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase5:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase6:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase7:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
LSetSides:
		faddp st(2),st(0)
		fcomp dword ptr[12+edx]
		xor ecx,ecx
		fnstsw ax
		fcomp dword ptr[12+edx]
		and ah,1
		xor ah,1
		add cl,ah
		fnstsw ax
		and ah,1
		add ah,ah
		add cl,ah
		pop ebx
		mov eax,ecx
		ret
Lerror:
		int 3
	}
	Com_Error( ERR_DROP, "\x15" "BoxOnPlaneSide: invalid signbits for plane" );
	/* Invalid plane sign bits indicate a programming error. */
	__asm int 3
}

#pragma warning( default: 4035 )
#endif
#endif


/*
=============
BoxDistSqrdExceeds

Measure squared distance from the origin argument to the nearest box point.
An axis whose interval straddles that point contributes zero distance.
Compare the accumulated distance with the supplied squared radius.
=============
*/
int BoxDistSqrdExceeds( const vec3_t boxMins, const vec3_t boxMaxs, const vec3_t origin, float radiusSquared ) {
	int i;
	vec3_t mins, maxs;
	float dist;
	float minSquared, maxSquared;

	VectorSubtract( boxMins, origin, mins );
	VectorSubtract( boxMaxs, origin, maxs );
	dist = 0;
	for ( i = 0; i < 3; i++ ) {
		if ( mins[i] * maxs[i] <= 0 ) continue;
		minSquared = mins[i] * mins[i];
		maxSquared = maxs[i] * maxs[i];
		dist += maxSquared < minSquared ? maxSquared : minSquared;
	}
	return dist > radiusSquared ? 1 : 0;
}

/*
=============
NormalToLatLong

Encode a normal as quantized latitude/longitude bytes, handling the poles.
=============
*/
void NormalToLatLong( const vec3_t normal, byte bytes[2] ) {
	// check for singularities
	if ( normal[0] == 0 && normal[1] == 0 ) {
		if ( normal[2] > 0 ) {
			bytes[0] = 0;
			bytes[1] = 0;		// lat = 0, long = 0
		} else {
			bytes[0] = 128;
			bytes[1] = 0;		// lat = 0, long = 128
		}
	} else {
		byte a, b;

		a = (int)( RAD2DEG( atan2( normal[1], normal[0] ) ) * (255.0f / 360.0f ) );

		b = (int)( RAD2DEG( acos( normal[2] ) ) * ( 255.0f / 360.0f ) );

		bytes[0] = b;	// longitude
		bytes[1] = a;	// latitude
	}
}

/*
=============
Vec10Copy

Copy all ten scalar components without interpreting their packed format.
=============
*/
void Vec10Copy( vec_t *in, vec_t *out ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
	out[4] = in[4];
	out[5] = in[5];
	out[6] = in[6];
	out[7] = in[7];
	out[8] = in[8];
	out[9] = in[9];
}

/*
=============
Q_rint

Round by flooring the input plus one half; negative ties round upward.
=============
*/
vec_t Q_rint (vec_t in)
{
	return floor (in + 0.5f);
}

/*
=============
ColorNormalize

Scale RGB by its maximum component; a zero input becomes white.
=============
*/
vec_t ColorNormalize( const vec3_t in, vec3_t out ) {
	float	max, scale;

	max = in[0];
	if (in[1] > max)
		max = in[1];
	if (in[2] > max)
		max = in[2];

	if (max == 0) {
		out[0] = out[1] = out[2] = 1.0;
		return 0;
	}

	scale = 1.0f / max;

	VectorScale (in, scale, out);

	return max;
}

/*
=============
VectorRotateAngles

Apply the three component-axis rotations in order using degree angles.
The index table selects the two coordinates affected by each rotation.
Each stage copies its result before applying the next rotation.
=============
*/
void VectorRotateAngles( const vec3_t in, const vec3_t angles, vec3_t out ) {
	vec3_t vWork, va;
	int nIndex[3][2];
	int i;
	double angle, c, s;

	VectorCopy( in, va );
	VectorCopy( va, vWork );
	nIndex[0][0] = 1; nIndex[0][1] = 2;
	nIndex[1][0] = 2; nIndex[1][1] = 0;
	nIndex[2][0] = 0; nIndex[2][1] = 1;

	for ( i = 0; i < 3; i++ ) {
		if ( angles[i] != 0.0f ) {
			angle = angles[i] * M_PI / 180.0;
			FastSinCosDouble( angle, &s, &c );
			vWork[nIndex[i][0]] = (float)(va[nIndex[i][0]] * c - va[nIndex[i][1]] * s);
			vWork[nIndex[i][1]] = (float)(va[nIndex[i][0]] * s + va[nIndex[i][1]] * c);
		}
		VectorCopy( vWork, va );
	}
	VectorCopy( vWork, out );
}

/*
=============
FastSinCosDouble

Use the x87 paired sine/cosine operation and store both double results.
=============
*/
__inline void FastSinCosDouble( double radians, double *sinOut, double *cosOut ) {
	__asm {
		push eax
		push ecx
		fld radians
		fsincos
		mov ecx, cosOut
		mov eax, sinOut
		fstp qword ptr [ecx]
		fstp qword ptr [eax]
		pop ecx
		pop eax
	}
}

/*
=============
VectorRotateAnglesAroundPoint

Translate to the pivot, rotate the displacement, then translate back.
=============
*/
void VectorRotateAnglesAroundPoint( const vec3_t point, const vec3_t angles,
							   const vec3_t pivot, vec3_t out ) {
	vec3_t vDelta, vRotatedDelta;

	VectorSubtract( point, pivot, vDelta );
	VectorRotateAngles( vDelta, angles, vRotatedDelta );
	VectorAdd( vRotatedDelta, pivot, out );
}

/*
=============
VectorPolar

Build a radius-scaled direction using the same angle for both polar inputs.
=============
*/
void VectorPolar( vec3_t out, float radius, float angle ) {
	float st, ct, sp, cp;
	FastSinCos( angle, &st, &ct );
	FastSinCos( angle, &sp, &cp );
	out[0] = radius * ct * cp;
	out[1] = radius * st * cp;
	out[2] = radius * sp;
}

/*
=============
VectorSnap

Round every coordinate with the same half-up rule as Q_rint.
=============
*/
void VectorSnap( vec3_t v ) {
	v[0] = floor( v[0] + 0.5f );
	v[1] = floor( v[1] + 0.5f );
	v[2] = floor( v[2] + 0.5f );
}

/*
=============
_Vector5Add

Add corresponding components of two five-component vectors.
=============
*/
void _Vector5Add (vec5_t va, vec5_t vb, vec5_t out)
{
	out[0] = va[0]+vb[0];
	out[1] = va[1]+vb[1];
	out[2] = va[2]+vb[2];
	out[3] = va[3]+vb[3];
	out[4] = va[4]+vb[4];
}

/*
=============
_Vector5Scale

Scale all five vector components by a single scalar.
=============
*/
void _Vector5Scale (vec5_t v, vec_t scale, vec5_t out)
{
	out[0] = v[0] * scale;
	out[1] = v[1] * scale;
	out[2] = v[2] * scale;
	out[3] = v[3] * scale;
	out[4] = v[4] * scale;
}

/*
=============
_Vector53Copy

Copy the first three components through the ordinary VectorCopy macro.
=============
*/
void _Vector53Copy( const vec3_t in, vec3_t out ) {
	VectorCopy( in, out );
}

/*
=============
RoundFloat

Round to a decimal precision by splitting scaled integer/fractional parts.
=============
*/
float RoundFloat( float value, int places ) {
	double dValue, frac;

	dValue = value * pow( 10.0, places );
	frac = modf( dValue, &dValue );
	if ( frac >= 0.5 ) {
		dValue += 1.0;
	} else if ( frac <= -0.5 ) {
		dValue += -1.0;
	}
	return (float)( pow( 0.1, places ) * dValue );
}

/*
=============
PitchForYawOnNormal

Project a horizontal heading onto the supplied plane and recover its pitch.
=============
*/
float PitchForYawOnNormal( float yaw, const vec3_t vNormal ) {
	vec3_t vForward;
	vec3_t vProjected;

	IW_ASSERT( vNormal[0] || vNormal[1] || vNormal[2] );

	YawVectors( yaw, vForward, NULL );
	ProjectPointOnPlane( vProjected, vForward, vNormal );
	return vectopitch( vProjected );
}

/*
=============
Rand_Init
=============
*/
void Rand_Init( int seed ) {
	com_randSeed = seed;
}

/*
=============
flrand
=============
*/
float flrand( float lo, float hi ) {
	float value;

	com_randSeed = 214013 * com_randSeed + 2531011;
	value = (unsigned)com_randSeed >> 17;
	value = ( hi - lo ) * value / 32768.0f + lo;
	return value;
}

/*
=============
irand
=============
*/
int irand( int lo, int hi ) {
	int value;
	com_randSeed = 214013 * com_randSeed + 2531011;
	value = (unsigned)com_randSeed >> 17;
	value = ( value * ( hi - lo ) >> 15 ) + lo;
	return value;
}

/*
=============
Q_Noise
=============
*/
float Q_Noise( float freqA, float freqB, float timeMs ) {
	return (float)( sin( timeMs / 1000.0 * freqA * M_PI * 2 ) *
		cos( timeMs / 1000.0 * freqB * M_PI * 2 ) );
}

