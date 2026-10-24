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

/* Shared byte representation, text, diagnostic and information-string helpers. */

#ifndef UO_XMODEL_Q_SHARED_H
#define UO_XMODEL_Q_SHARED_H

#include "common/cmdlib.h"
#include "common/mathlib.h"
#include "q_assert.h"

/* Quake shared-code byte representation; passed and returned by value. */
typedef struct {
	byte b0;
	byte b1;
	byte b2;
	byte b3;
	byte b4;
	byte b5;
	byte b6;
	byte b7;
} qint64;

/* Shared byte-order and text helpers (q_shared.c). */
qint64 Long64Swap( qint64 ll );
int Q_islower( int c );
int Q_stricmpn( const char *s1, const char *s2, int n );

#define MAX_INFO_STRING 1024
#define ERR_DROP 1

/* Engine diagnostics (com_print.cpp). */
void Com_Error( int code, const char *format, ... );
void Com_Printf( const char *format, ... );

/* Bounded text and information strings (q_shared.c). */
void Com_sprintf( char *dest, int size, const char *format, ... );
void Info_RemoveKey( char *s, const char *key );

#define VectorMA( v, s, b, o ) ( (o)[0] = (b)[0] * (s) + (v)[0], (o)[1] = (b)[1] * (s) + (v)[1], (o)[2] = (b)[2] * (s) + (v)[2] )
#define VectorSet(v,x,y,z) ((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);


/* Public text, configuration and orientation records. */
char *va(char *, ...);
enum { ERR_FATAL = 0 };
enum { BIG_INFO_STRING = 8192, BIG_INFO_KEY = 8192, BIG_INFO_VALUE = 8192 };

enum { MAX_INFO_KEY = 1024, MAX_INFO_VALUE = 1024 };
enum { MAX_QPATH = 64 };

typedef struct {
	vec3_t origin, axis[3];
} orientation_t;

int LongNoSwap( int l );
float Q_fabs( float f );
void Q_strncpyz(char *dest, const char *src, int destsize);

typedef enum {
	CS_FIELD_VECTOR = 0,
	CS_FIELD_STRING1024 = 1, CS_FIELD_STRING64 = 2, CS_FIELD_STRING256 = 3,
	CS_FIELD_INT = 4, CS_FIELD_QBOOLEAN = 5,
	CS_FIELD_FLOAT = 6, CS_FIELD_MILLISECONDS = 7,
	CS_FIELD_COUNT
} csFieldType_t;

typedef struct {
	const char *name;
	int offset, type;
} csField_t;

/* Color escapes accept all ten decimal color indices. */
#define Q_IsColorString(p) ((p) && (p)[0] == '^' && (p)[1] && (p)[1] != '^' && (p)[1] >= '0' && (p)[1] <= '9')

unsigned char ColorIndex( unsigned char c );
float Com_Clamp( float min, float max, float value );
char *Com_SkipPath( char *pathname );
void Com_StripExtension( const char *in, char *out );
void Com_StripFilename( char *in, char *out );
void Com_DefaultExtension( char *path, int maxSize, const char *extension );
qboolean Com_BitCheck( const int array[], int bitNum );
void Com_BitSet( int array[], int bitNum );
void Com_BitClear( int array[], int bitNum );
short   BigShort( short l );
int     BigLong( int l );
qint64  BigLong64( qint64 l );
qint64  LittleLong64( qint64 l );
float   BigFloat( float l );
short   ShortSwap (short l);
short	ShortNoSwap (short l);
int    LongSwap (int l);
int	LongNoSwap (int l);
qint64 Long64Swap (qint64 ll);
qint64 Long64NoSwap (qint64 ll);
float FloatSwap( float f );
float FloatNoSwap( float f );
void Swap_Init( void );
int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
int Q_isnumeric( int c );
int Q_isalphanumeric( int c );
int Q_isforfilename( int c );
char *Q_strrchr( const char *string, int c );
void Q_strncpyz(char *dest, const char *src, int destsize);
int Q_stricmpn( const char *s1, const char *s2, int n );
int Q_strncmp (const char *s1, const char *s2, int n);
int Q_stricmp (const char *s1, const char *s2);
char *Q_strlwr( char *s1 );
char *Q_strupr( char *s1 );
void Q_strcat( char *dest, int size, const char *src );
int Q_DrawStrlen( const char *string );
char *Q_CleanStr( char *string );
char Q_CleanCharacter( char c );
void Com_sprintf( char *dest, int size, const char *fmt, ... );
int Q_strncasecmp( const char *s1, const char *s2, int n );
int Q_strcasecmp( char *s1, char *s2 );
char *va( char *format, ... );
float   *tv( float x, float y, float z );
char *Info_ValueForKey( const char *s, const char *key );
void Info_NextPair( const char **head, char *key, char *value );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_Big( char *s, const char *key );
qboolean Info_Validate( const char *s );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean ParseConfigStringToStruct( void *structBase, const csField_t *fields, int numFields,
		const char *s, int numCustomTypes,
		qboolean ( *parseSpecialFieldType )( void *base, const char *value, int type ),
		void ( *vectorHandler )( void *dest, const char *value ) );
float GetLeanFraction( float f );
float Q_fabs( float f );
float UnGetLeanFraction( float fFrac );
void AddLeanToPosition( float *pos, float yaw, float leanFrac, float leanDist, float scale );
void OrientationPosToWorldPos( const orientation_t *orient, const vec3_t pos, vec3_t out );
void OrientationDirToWorldDir( const orientation_t *orient, const vec3_t dir, vec3_t out );
void OrientationPosFromWorldPos( const orientation_t *orient, const vec3_t worldPos, vec3_t out );
void OrientationDirFromWorldDir( const orientation_t *orient, const vec3_t worldDir, vec3_t out );

#endif
