/* Public vector, matrix, angle and bounds declarations. */
#ifndef UO_XANIM_COM_MATH_H
#define UO_XANIM_COM_MATH_H

#include "../../universal/com_math.h"

#define IS_NAN(x) (((*(int *)&(x)) & 0x7f800000) == 0x7f800000)

typedef struct cplane_s {
	vec3_t normal;
	float dist;
	byte type;
	byte signbits;
} cplane_t;


#define VECTOR_COMPARE_EPSILON 0.001f
#define MAX_BOUNDS_COORD 262144

#define ERR_DROP 1
void Com_Error( int code, const char *fmt, ... );
#ifndef M_PI
#define M_PI 3.14159265358979323846f /* matches value in gcc v2 math.h */
#endif
#undef DEG2RAD
#undef RAD2DEG
#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )
#define VectorMA( v, s, b, o )     ((o)[0]=(v)[0]+(b)[0]*(s),      (o)[1]=(v)[1]+(b)[1]*(s),      (o)[2]=(v)[2]+(b)[2]*(s))
#undef VectorSubtract
#define VectorSubtract( a, b, c ) ((c)[0]=(a)[0]-(b)[0], (c)[1]=(a)[1]-(b)[1], (c)[2]=(a)[2]-(b)[2])
#undef VectorCopy
#define VectorCopy( a, b ) ((b)[0]=(a)[0], (b)[1]=(a)[1], (b)[2]=(a)[2])
#undef VectorAdd
#define VectorAdd( a, b, c ) ((c)[0]=(a)[0]+(b)[0], (c)[1]=(a)[1]+(b)[1], (c)[2]=(a)[2]+(b)[2])
#undef VectorScale
#define VectorScale( a, b, c ) ((c)[0]=(b)*(a)[0], (c)[1]=(b)*(a)[1], (c)[2]=(b)*(a)[2])
#define Vector2Subtract( a, b, c ) ((c)[0]=(a)[0]-(b)[0], (c)[1]=(a)[1]-(b)[1])

typedef vec_t vec5_t[5];
#define NUMVERTEXNORMALS 162

int Q_rand( int *seed );
float Q_random( int *seed );
float AngleNormalize360( float angle );
float AngleNormalize180( float angle );
void RotateAroundDirection( vec3_t axis[3], float yaw );
float NormalizeColor( const vec3_t in, vec3_t out );
float LerpAngle( float from, float to, float frac );
float AngleSubtract( float a1, float a2 );
void AnglesSubtract( const vec3_t v1, const vec3_t v2, vec3_t v3 );
float AngleNormalize360Accurate( float angle );
float AngleNormalize180Accurate( float angle );
float Q_crandom( int *seed );
int Q_log2( int val );
float Q_acos( float c );
signed char ClampChar( int i );
signed short ClampShort( int i );
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void NormalToLatLong( const vec3_t normal, byte bytes[2] );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
void Vec10Copy( vec_t *in, vec_t *out );
void MatrixTransformVector( vec3_t v, float r[3][3], vec3_t d );
float _VectorLength( const vec3_t v );
vec_t Q_rint (vec_t in);
void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
vec_t _DotProduct (vec3_t v1, vec3_t v2);
void _VectorSubtract (vec3_t va, vec3_t vb, vec3_t out);
void _VectorAdd (vec3_t va, vec3_t vb, vec3_t out);
void _VectorCopy (vec3_t in, vec3_t out);
void _VectorScale (vec3_t v, vec_t scale, vec3_t out);
vec_t ColorNormalize( const vec3_t in, vec3_t out );
void VectorInverse (vec3_t v);
void ClearBounds (vec3_t mins, vec3_t maxs);
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );
void ExpandBounds( const vec3_t mins, const vec3_t maxs, vec3_t outMins, vec3_t outMaxs );
void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void PerpendicularVector( vec3_t dst, const vec3_t src );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							 float degrees );
void _Vector5Add (vec5_t va, vec5_t vb, vec5_t out);
void _Vector5Scale (vec5_t v, vec_t scale, vec5_t out);
__inline void FastSinCos( float radians, float *sinOut, float *cosOut );
__inline void FastSinCosDouble( double radians, double *sinOut, double *cosOut );
qboolean VectorCompareEpsilon( const vec3_t p1, const vec3_t p2 );
void YawVectors( float yaw, vec3_t forward, vec3_t right );
void VectorAngleMultiply( vec3_t v, float degrees );
void VectorPolar( vec3_t out, float radius, float angle );
void RollToQuaternion( float degrees, vec4_t q );
void gunrandom( float *outX, float *outY );
void MatrixMultiplyEquals( const float left[3][3], float inout[3][3] );
float vectopitch( const vec3_t vec );
float vectosignedpitch( const vec3_t vec );
void vectosignedangles( const vec3_t value, vec3_t angles );
void vectoangles( const vec3_t value, vec3_t angles );
float VectorNormalize2D( float v[2] );
float VectorNormalize4D( vec4_t v );
float VectorNormalize2( const vec3_t v, vec3_t out );
float RoundFloat( float value, int places );
float QuatEigenTrace( const vec4_t q );
void QuatInverse( const vec4_t in, vec4_t out );
void ConvertQuatToMat( float quatAndMatrix[9] );
void MatrixTransposeTransformVector43( const vec3_t in, const float m[4][3], vec3_t out );
void MatrixTranspose( const float in[3][3], float out[3][3] );
void MatrixInverseOrthogonal43( const float in[4][3], float out[4][3] );
vec_t VectorNormalize( vec3_t v );
void VectorRotateAnglesAroundPoint( const vec3_t point, const vec3_t angles,
							   const vec3_t pivot, vec3_t out );
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
float VectorDistance( const vec3_t p1, const vec3_t p2 );
float VectorDistanceSquared( const vec3_t p1, const vec3_t p2 );
float VectorDistance2D( const vec3_t p1, const vec3_t p2 );
float VectorDistanceSquared2D( const vec3_t p1, const vec3_t p2 );
void MatrixMultiply34( const float left[3][4], const float right[3][4], float out[3][4] );
void MatrixMultiply43( const float left[4][3], const float right[4][3], float out[4][3] );
void DObjSkelMatrixMultiply43( const float left[4][4], const float right[4][3], float out[4][3] );
void DObjSkel2MatrixMultiply43( const float left[4][4], const float right[4][3], float out[4][4] );
void MatrixTransformVector43( const vec3_t in, const float m[4][3], vec3_t out );
void DObjSkelMatrixTransformVector43( const vec3_t in, const float m[4][4], vec3_t out );
void VectorSnap( vec3_t v );
void GetPerpendicularViewVector( const vec3_t a, const vec3_t origin,
						 const vec3_t b, vec3_t normal );
void ProjectPointOntoVector( const vec3_t point, const vec3_t origin,
							 const vec3_t linePoint, vec3_t out );
void AxisClear( vec3_t axis[3] );
float QuatRatioEigenTrace( const vec4_t a, const vec4_t b );
float AngleEigenTrace( float degrees );
byte DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );
void SetPlaneSignbits( cplane_t *out );
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void YawToAxis( float yaw, vec3_t axis[3] );
float PitchForYawOnNormal( float yaw, const vec3_t normal );
void Rand_Init( int seed );
float flrand( float lo, float hi );
int irand( int lo, int hi );
void Vector4Scale( const vec4_t in, float scale, vec4_t out );
void _VectorMA( const vec3_t a, float scale, const vec3_t b, vec3_t out );
float VectorMax( const vec3_t v );
void QuatMultiply( const vec4_t a, const vec4_t b, vec4_t out );
void MatrixInverse44( const float in[16], float out[16] );
void MatrixTransformVector43Equals( vec3_t v, const float matrix[4][3] );
void _Vector53Copy( const vec3_t in, vec3_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );
void AxisCopy( vec3_t in[3], vec3_t out[3] );
float AngleDelta( float angle1, float angle2 );
float vectoyaw( const vec3_t vec );
float vectosignedyaw( const vec3_t vec );
unsigned ColorBytes3( float length, float g, float b );
unsigned ColorBytes4( float length, float g, float b, float a );
float RotationToYaw( const float v[2] );
void VectorNormalizeFast( vec3_t v );
void CrossProductUp( const vec3_t in, vec3_t out );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
void MatrixInverse( const float in[9], float out[9] );
void AxisToAngles( const vec3_t axis[3], vec3_t angles );
void Axis4ToAngles( const float matrix[3][4], vec3_t angles );
void AxisToSignedAngles( const vec3_t axis[3], vec3_t angles );
void VectorRotateAngles( const vec3_t in, const vec3_t angles, vec3_t out );
int BoxOnPlaneSide( vec3_t emins, vec3_t emaxs, struct cplane_s *p );

__inline float Q_rsqrt( float number );

#endif
