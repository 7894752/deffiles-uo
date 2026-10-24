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

/* Shared text, path, byte-order and information-string helpers. */

#include "q_shared.h"

const vec4_t colorBlack     = { 0.00f, 0.00f, 0.00f, 1.0f };
const vec4_t colorRed       = { 1.00f, 0.00f, 0.00f, 1.0f };
const vec4_t colorGreen     = { 0.00f, 1.00f, 0.00f, 1.0f };
const vec4_t colorLtGreen   = { 0.00f, 0.70f, 0.00f, 1.0f };
const vec4_t colorBlue      = { 0.00f, 0.00f, 1.00f, 1.0f };
const vec4_t colorYellow    = { 1.00f, 1.00f, 0.00f, 1.0f };
const vec4_t colorLtYellow  = { 0.75f, 0.75f, 0.00f, 1.0f };
const vec4_t colorMdYellow  = { 0.50f, 0.50f, 0.00f, 1.0f };
const vec4_t colorMagenta   = { 1.00f, 0.00f, 1.00f, 1.0f };
const vec4_t colorCyan      = { 0.00f, 1.00f, 1.00f, 1.0f };
const vec4_t colorLtCyan    = { 0.00f, 0.75f, 0.75f, 1.0f };
const vec4_t colorMdCyan    = { 0.00f, 0.50f, 0.50f, 1.0f };
const vec4_t colorDkCyan    = { 0.00f, 0.25f, 0.25f, 1.0f };
const vec4_t colorWhite     = { 1.00f, 1.00f, 1.00f, 1.0f };
const vec4_t colorLtGrey    = { 0.75f, 0.75f, 0.75f, 1.0f };
const vec4_t colorMdGrey    = { 0.50f, 0.50f, 0.50f, 1.0f };
const vec4_t colorDkGrey    = { 0.25f, 0.25f, 0.25f, 1.0f };
const vec4_t colorOrange    = { 1.00f, 0.70f, 0.00f, 1.0f };
const vec4_t colorLtOrange  = { 0.75f, 0.525f, 0.00f, 1.0f };
extern qboolean com_errorEntered;

enum { MAX_VA_STRING = 32000 };
/*
=============
ColorIndex
=============
*/
unsigned char ColorIndex( unsigned char c ) {
	unsigned char index = c - '0';
	return index < 10 ? index : 7;
}

static short ( *_BigShort )( short l ), ( *_LittleShort )( short l );
static int ( *_BigLong )( int l ), ( *_LittleLong )( int l );
static qint64 ( *_BigLong64 )( qint64 l ), ( *_LittleLong64 )( qint64 l );
static float ( *_BigFloat )( float l ), ( *_LittleFloat )( float l );

/*
============
Com_Clamp
============
*/
float Com_Clamp( float min, float max, float value ) {
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
============
Com_SkipPath
============
*/
char *Com_SkipPath( char *pathname ) {
	char *last = pathname;
	while ( *pathname ) {
		if ( *pathname == '/' ) last = pathname + 1;
		pathname++;
	}
	return last;
}

/*
============
Com_StripExtension
============
*/
void Com_StripExtension( const char *in, char *out ) {
	while ( *in && *in != '.' ) {
		*out++ = *in++;
	}
	*out = 0;
}

/*
=============
Com_StripFilename
=============
*/
void Com_StripFilename( char *in, char *out ) {
	char *end;
	Q_strncpyz( out, in, strlen( in ) );
	end = Com_SkipPath( out );
	*end = 0;
}

/*
==================
Com_DefaultExtension
==================
*/
void Com_DefaultExtension( char *path, int maxSize, const char *extension ) {
	char	oldPath[MAX_QPATH];
	char    *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path) {
		if ( *src == '.' ) {
			return;                 // it has an extension
		}
		src--;
	}

	Q_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

/*
=============
Com_BitCheck
=============
*/
qboolean Com_BitCheck( const int array[], int bitNum ) { return ( ( array[bitNum >> 5] & ( 1 << ( bitNum & 31 ) ) ) != 0 ); }

/*
=============
Com_BitSet
=============
*/
void Com_BitSet( int array[], int bitNum ) { array[bitNum >> 5] |= ( 1 << ( bitNum & 31 ) ); }

/*
=============
Com_BitClear
=============
*/
void Com_BitClear( int array[], int bitNum ) { array[bitNum >> 5] &= ~( 1 << ( bitNum & 31 ) ); }


/*
=============
BigShort
=============
*/
short   BigShort( short l ) {return _BigShort( l );}


/*
=============
BigLong
=============
*/
int     BigLong( int l ) {return _BigLong( l );}

/*
=============
BigLong64
=============
*/
qint64  BigLong64( qint64 l ) {return _BigLong64( l );}

/*
=============
LittleLong64
=============
*/
qint64  LittleLong64( qint64 l ) {return _LittleLong64( l );}


/*
=============
BigFloat
=============
*/
float   BigFloat( float l ) {return _BigFloat( l );}

/*
=============
ShortSwap
=============
*/
short   ShortSwap (short l)
{
	byte b1 = l & 255, b2 = (l >> 8) & 255;
	return (b1 << 8) + b2;
}

/*
=============
ShortNoSwap
=============
*/
short	ShortNoSwap (short l)
{
	return l;
}

/*
=============
LongSwap
=============
*/
int    LongSwap (int l)
{
	byte b1 = l & 255, b2 = (l >> 8) & 255;
	byte b3 = (l >> 16) & 255, b4 = (l >> 24) & 255;
	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

/*
=============
LongNoSwap
=============
*/
int	LongNoSwap (int l)
{
	return l;
}

/*
=============
Long64Swap
=============
*/
qint64 Long64Swap (qint64 ll)
{
	qint64 result = { ll.b7, ll.b6, ll.b5, ll.b4, ll.b3, ll.b2, ll.b1, ll.b0 };
	return result;
}

/*
=============
Long64NoSwap
=============
*/
qint64 Long64NoSwap (qint64 ll)
{
	return ll;
}

/*
=============
FloatSwap
=============
*/
float FloatSwap( float f ) {
	union { float f; byte b[4]; } dat1, dat2;
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}

/*
=============
FloatNoSwap
=============
*/
float FloatNoSwap( float f ) { return f; }

/*
================
Swap_Init
================
*/
void Swap_Init( void ) {
	byte swaptest[2] = {1,0};

// set the byte swapping variables in a portable manner
	if ( *(short *)swaptest == 1 ) {
		_BigShort = ShortSwap;
		_LittleShort = ShortNoSwap;
		_BigLong = LongSwap;
		_LittleLong = LongNoSwap;
		_BigLong64 = Long64Swap;
		_LittleLong64 = Long64NoSwap;
		_BigFloat = FloatSwap;
		_LittleFloat = FloatNoSwap;
	} else
	{
		_BigShort = ShortNoSwap;
		_LittleShort = ShortSwap;
		_BigLong = LongNoSwap;
		_LittleLong = LongSwap;
		_BigLong64 = Long64NoSwap;
		_LittleLong64 = Long64Swap;
		_BigFloat = FloatNoSwap;
		_LittleFloat = FloatSwap;
	}
}

/*
=============
Q_isprint
=============
*/
int Q_isprint( int c ) {
	if ( c >= 0x20 && c <= 0x7E ) return 1;
	return 0;
}

/*
=============
Q_islower
=============
*/
int Q_islower( int c ) {
	if ( c >= 'a' && c <= 'z' ) return 1;
	return 0;
}

/*
=============
Q_isupper
=============
*/
int Q_isupper( int c ) {
	if ( c >= 'A' && c <= 'Z' ) return 1;
	return 0;
}

/*
=============
Q_isalpha
=============
*/
int Q_isalpha( int c ) {
	if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ) return 1;
	return 0;
}

/*
=============
Q_isnumeric
=============
*/
int Q_isnumeric( int c ) {
	if ( c >= '0' && c <= '9' ) return 1;
	return 0;
}

/*
=============
Q_isalphanumeric
=============
*/
int Q_isalphanumeric( int c ) {
	if ( Q_isalpha( c ) || Q_isnumeric( c ) ) return 1;
	return 0;
}

/*
=============
Q_isforfilename
=============
*/
int Q_isforfilename( int c ) {
	/* Spaces are excluded from names. */
	if ( (Q_isalphanumeric( c ) || c == '_' || c == '-') && c != ' ' ) return 1;
	return 0;
}

/*
=============
Q_strrchr
=============
*/
char *Q_strrchr( const char *string, int c ) {
	char cc = c;
	char *s;
	char *sp = NULL;
	s = (char *)string;
	while ( *s ) {
		if ( *s == cc ) sp = s;
		s++;
	}
	if ( cc == 0 ) sp = s;
	return sp;
}

/*
=============
Q_strncpyz
 
Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz(char *dest, const char *src, int destsize) {
    Q_ASSERT(src, "src");
    Q_ASSERT(dest, "dest");
    Q_ASSERT((destsize >= 1), va("%s\n\t(destsize) = %i", "(destsize >= 1)", destsize));
    strncpy(dest, src, destsize - 1);
    dest[destsize - 1] = 0;
}

/*
Q_stricmpn
*/
int Q_stricmpn( const char *s1, const char *s2, int n ) {
	int c1, c2;
	do {
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;       // strings are equal until end point
		}

		if ( c1 != c2 ) {
			if ( Q_islower( c1 ) ) {
				c1 -= ( 'a' - 'A' );
			}
			if ( Q_islower( c2 ) ) {
				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while ( c1 );

	return 0;       // strings are equal
}

/*
Q_strncmp
*/
int Q_strncmp (const char *s1, const char *s2, int n) {
	int		c1, c2;
	do {
		c1 = *s1++;
		c2 = *s2++;
		if (!n--) {
			return 0;		// strings are equal until end point
		}
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	return 0;		// strings are equal
}

/*
Q_stricmp
*/
int Q_stricmp (const char *s1, const char *s2) {
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}

/*
Q_strlwr
*/
char *Q_strlwr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
    return s1;
}

/*
Q_strupr
*/
char *Q_strupr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = toupper(*s);
		s++;
	}
    return s1;
}

/*
Q_strcat
*/
void Q_strcat( char *dest, int size, const char *src ) {
	int l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "\x15" "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}

int Q_DrawStrlen( const char *string ) {
	const char *p = string;
	int len = 0;
	while ( *p ) {
		if ( Q_IsColorString( p ) ) {
			p += 2;
		} else {
			len++;
			p++;
		}
	}
	return len;
}

/*
Q_CleanStr
*/
char *Q_CleanStr( char *string ) {
	char *d;
	char *s;
	char c;

	s = string;
	d = string;
	while ( ( c = *s ) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		} else if ( c >= 0x20 && c <= 0x7E ) {
			*d++ = c;
		}
		s++;
	}
	*d = 0;
	return string;
}

/*
Q_CleanCharacter
*/
char Q_CleanCharacter( char c ) {
	return c;
}

/*
Com_sprintf
*/
void Com_sprintf( char *dest, int size, const char *fmt, ... ) {
	int len;
	va_list argptr;

	va_start( argptr, fmt );
	len = _vsnprintf( dest, size, fmt, argptr );
	va_end( argptr );
	dest[size - 1] = 0;
	Q_ASSERT( len >= 0, va("%s\n\t%s", "len >= 0", size <= 1024 ? dest : "<string too long>") );
}

/*
=============
Q_strncasecmp
=============
*/
int Q_strncasecmp( const char *s1, const char *s2, int n ) {
	int c1, c2;

	do
	{
		c1 = *s1++;
		c2 = *s2++;

		if ( !n-- ) {
			return 0;       // strings are equal until end point

		}
		if ( c1 != c2 ) {
			if ( Q_islower( c1 ) ) {
				c1 -= ( 'a' - 'A' );
			}
			if ( Q_islower( c2 ) ) {
				c2 -= ( 'a' - 'A' );
			}
			if ( c1 != c2 ) {
				return -1;      // strings not equal
			}
		}
	} while ( c1 );

	return 0;       // strings are equal
}

/*
=============
Q_strcasecmp
=============
*/
int Q_strcasecmp( char *s1, char *s2 ) {
	return Q_strncasecmp( s1, s2, 99999 );
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday
============
*/
char *va( char *format, ... ) {
	va_list argptr;
	static char buffer[MAX_VA_STRING];
	static char vabuf[MAX_VA_STRING];
	static int index = 0;
	char *buf;
	int len;

	va_start( argptr, format );
	len = _vsnprintf( buffer, MAX_VA_STRING, format, argptr );
	buffer[MAX_VA_STRING - 1] = 0;
	va_end( argptr );

	if ( len < 0 || len >= MAX_VA_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Attempted to overrun string in call to va()\n" );
	}
	if ( len + index >= MAX_VA_STRING - 1 ) {
		index = 0;
	}

	buf = &vabuf[index];
	memcpy( buf, buffer, len + 1 );
	index += len + 1;
	return buf;
}

/*
=============
tv
=============
*/
float   *tv( float x, float y, float z ) {
	static int index;
	static vec3_t vecs[8];
	float   *v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = ( index + 1 ) & 7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
Entries use \key\value syntax; key comparisons ignore case.
Info_ValueForKey alternates two buffers so two returned values can coexist.
Info_NextPair advances a caller-owned cursor and copies one pair.
The normal and Big helpers use their corresponding capacity limits.
Setters remove an old entry before appending its replacement.
Backslashes, semicolons and quotes are discarded from replacement values.
An empty sanitized value therefore removes the key without adding a pair.
*/
/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key ) {
	char pkey[BIG_INFO_KEY];
	static char buf[2][BIG_INFO_VALUE];
	static int valueindex = 0;
	char *o;

	if ( !s || !key ) {
		return "";
	}
	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if ( *s == '\\' ) {
		s++;
	}
	for ( ;; ) {
		o = pkey;
		while ( *s != '\\' ) {
			if ( !*s ) {
				return "";
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = buf[valueindex];
		while ( *s != '\\' && *s ) {
			*o++ = *s++;
		}
		*o = 0;

		if ( !Q_stricmp( key, pkey ) ) {
			return buf[valueindex];
		}
		if ( !*s ) {
			break;
		}
		s++;
	}
	return "";
}

/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char *key, char *value ) {
	char	*o;
	const char	*s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}

/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char *start;
	char pkey[MAX_INFO_KEY];
	char value[MAX_INFO_VALUE];
	char *o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_RemoveKey: oversize infostring" );
	}
	if ( strchr( key, '\\' ) ) {
		return;
	}

	for ( ;; ) {
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' ) {
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s ) {
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );
			return;
		}
		if ( !*s ) {
			return;
		}
	}
}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
	char *start;
	char pkey[BIG_INFO_KEY];
	char value[BIG_INFO_VALUE];
	char *o;

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_RemoveKey_Big: oversize infostring" );
	}
	if ( strchr( key, '\\' ) ) {
		return;
	}

	for ( ;; ) {
		start = s;
		if ( *s == '\\' ) {
			s++;
		}
		o = pkey;
		while ( *s != '\\' ) {
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while ( *s != '\\' && *s ) {
			if ( !*s ) {
				return;
			}
			*o++ = *s++;
		}
		*o = 0;

		if ( !strcmp( key, pkey ) ) {
			strcpy( start, s );
			return;
		}
		if ( !*s ) {
			return;
		}
	}
}

/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
	if ( strchr( s, '\"' ) ) {
		return qfalse;
	}
	if ( strchr( s, ';' ) ) {
		return qfalse;
	}
	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char newi[MAX_INFO_STRING];
	char cleanValue[MAX_INFO_STRING];
	int i, j;



	IW_ASSERT( value );

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_SetValueForKey: oversize infostring" );
	}

	j = 0;
	for ( i = 0 ; i < MAX_INFO_STRING - 1 ; i++ ) {
		char c = value[i];
		if ( !c ) break;
		if ( c == '\\' || c == ';' || c == '"' ) continue;

		IW_ASSERT( j < MAX_INFO_STRING );
		cleanValue[j] = c;
		j++;

	}
	IW_ASSERT( j < MAX_INFO_STRING );
	cleanValue[j] = 0;
	if ( strchr( key, '\\' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \\\nkey: '%s'\nvalue: '%s'", key, value );
	}
	if ( strchr( key, ';' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'", key, value );
	}
	if ( strchr( key, '\"' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'", key, value );
	}
	Info_RemoveKey( s, key );
	if ( !cleanValue[0] ) {
		return;
	}
	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, cleanValue );

	if ( strlen( newi ) + strlen( s ) > MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n", key, value, s );
	}

	strcat( s, newi );
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char newi[BIG_INFO_STRING];
	char cleanValue[BIG_INFO_STRING];
	int i, j;



	IW_ASSERT( value );

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "Info_SetValueForKey: oversize infostring" );
	}

	j = 0;
	for ( i = 0; i < BIG_INFO_STRING - 1; i++ ) {
		char c = value[i];
		if ( !c ) break;
		if ( c == '\\' || c == ';' || c == '"' ) continue;

		IW_ASSERT( j < BIG_INFO_STRING );
		cleanValue[j] = c;
		j++;

	}
	IW_ASSERT( j < BIG_INFO_STRING );
	cleanValue[j] = 0;

	if ( strchr( key, '\\' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \\\nkey: '%s'\nvalue: '%s'", key, value );
	}
	if ( strchr( key, ';' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a semicolon\nkey: '%s'\nvalue: '%s'", key, value );
	}
	if ( strchr( key, '"' ) ) {
		Com_Error( ERR_DROP, "\x15" "Can't use keys with a \"\nkey: '%s'\nvalue: '%s'", key, value );
	}

	Info_RemoveKey_Big( s, key );
	if ( !cleanValue[0] ) {
		return;
	}

	Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, cleanValue );
	if ( strlen( newi ) + strlen( s ) > BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "\x15" "BIG Info string length exceeded\nkey: '%s'\nvalue: '%s'\nInfo string:\n%s\n",
				   key, value, s );
	}

	strcat( s, newi );
}

/*
=============
ParseConfigStringToStruct
=============
*/
qboolean ParseConfigStringToStruct( void *structBase, const csField_t *fields, int numFields,
		const char *s, int numCustomTypes,
		qboolean ( *parseSpecialFieldType )( void *base, const char *value, int type ),
		void ( *vectorHandler )( void *dest, const char *value ) ) {
	int i;
	char *value;
	const csField_t *field;

	for ( i = 0, field = fields; i < numFields; i++, field++ ) {
		value = Info_ValueForKey( s, field->name );
		if ( !*value ) {
			continue;
		}

		if ( field->type < CS_FIELD_COUNT ) {
			switch ( field->type ) {
			case CS_FIELD_VECTOR:
				vectorHandler( (byte *)structBase + field->offset, value );
				break;
			case CS_FIELD_STRING1024:
				Q_strncpyz( (char *)( (byte *)structBase + field->offset ), value, 1024 );
				break;
			case CS_FIELD_STRING64:
				Q_strncpyz( (char *)( (byte *)structBase + field->offset ), value, 64 );
				break;
			case CS_FIELD_STRING256:
				Q_strncpyz( (char *)( (byte *)structBase + field->offset ), value, 256 );
				break;
			case CS_FIELD_INT:
				*(int *)( (byte *)structBase + field->offset ) = atoi( value );
				break;
			case CS_FIELD_QBOOLEAN:
				*(int *)( (byte *)structBase + field->offset ) = ( atoi( value ) != 0 );
				break;
			case CS_FIELD_FLOAT:
				*(float *)( (byte *)structBase + field->offset ) = (float)atof( value );
				break;
			case CS_FIELD_MILLISECONDS:
				*(int *)( (byte *)structBase + field->offset ) = (int)( (float)atof( value ) * 1000.0f );
				break;
			default:
				// Keep malformed built-in types separate from custom field types.

				if ( field->type < 0 ) {
					Q_ASSERT( com_errorEntered, va("Negative field type %i given to ParseConfigStringToStruct\n", field->type) );
				} else {
					Q_ASSERT( com_errorEntered, "ParseConfigStringToStruct is out of sync with the csParseFieldType_t enum list\n" );
				}
				break;
			}
		} else if ( numCustomTypes > 0 && field->type < numCustomTypes ) {
			IW_ASSERT( parseSpecialFieldType != NULL );
			if ( !parseSpecialFieldType( structBase, value, field->type ) ) {
				return qfalse;
			}
		} else {

			Q_ASSERT( com_errorEntered, va("Bad field type %i\n", field->type) );
			Com_Error( ERR_DROP, "\x15" "Bad field type %i\n", field->type );
		}
	}
	if ( i != numFields ) {
		return qfalse;
	}
	return qtrue;
}

/*
=============
GetLeanFraction
=============
*/
float GetLeanFraction( float f ) {
	return ( 2.0f - Q_fabs( f ) ) * f;
}

/*
=============
Q_fabs
=============
*/
float Q_fabs( float f ) {
	int tmp = *(int *)&f & 0x7FFFFFFF;
	return *(float *)&tmp;
}

/*
GetLeanFraction maps the signed lean control to a curved displacement:
    f -> (2 - |f|) f.
On its nonnegative branch, solving this quadratic gives the inverse:
    f = 1 - sqrt(1 - fFrac).
*/
/*
=============
UnGetLeanFraction
=============
*/
float UnGetLeanFraction( float fFrac ) {
	IW_ASSERT( fFrac >= 0 );
	IW_ASSERT( fFrac <= 1.f );
	return 1.0f - (float)sqrt( 1.0f - fFrac );
}

/*
=============
AddLeanToPosition
=============
*/
void AddLeanToPosition( float *pos, float yaw, float leanFrac, float leanDist, float scale ) {
	float frac;
	vec3_t vRight, vAng;
	if ( leanFrac != 0.0f ) {
		frac = GetLeanFraction( leanFrac );
		VectorSet( vAng, 0, yaw, frac * leanDist );
		AngleVectors( vAng, NULL, vRight, NULL );
		frac *= scale;
		VectorMA(
			pos, frac, vRight, pos
		);
	}
}

/*
=============
OrientationPosToWorldPos
=============
*/
void OrientationPosToWorldPos( const orientation_t *orient, const vec3_t pos, vec3_t out ) {
	out[0] = pos[0] * orient->axis[0][0] + orient->origin[0] + pos[1] * orient->axis[1][0] + pos[2] * orient->axis[2][0];
	out[1] = pos[0] * orient->axis[0][1] + orient->origin[1] + pos[1] * orient->axis[1][1] + pos[2] * orient->axis[2][1];
	out[2] = pos[0] * orient->axis[0][2] + orient->origin[2] + pos[1] * orient->axis[1][2] + pos[2] * orient->axis[2][2];
}

/*
=============
OrientationDirToWorldDir
=============
*/
void OrientationDirToWorldDir( const orientation_t *orient, const vec3_t dir, vec3_t out ) {
	out[0] = dir[0] * orient->axis[0][0] + dir[1] * orient->axis[1][0] + dir[2] * orient->axis[2][0];
	out[1] = dir[0] * orient->axis[0][1] + dir[1] * orient->axis[1][1] + dir[2] * orient->axis[2][1];
	out[2] = dir[0] * orient->axis[0][2] + dir[1] * orient->axis[1][2] + dir[2] * orient->axis[2][2];
}

/*
=============
OrientationPosFromWorldPos
=============
*/
void OrientationPosFromWorldPos( const orientation_t *orient, const vec3_t worldPos, vec3_t out ) {
	vec3_t dir;

	VectorSubtract( worldPos, orient->origin, dir );
	out[0] = DotProduct( dir, orient->axis[0] );
	out[1] = DotProduct( dir, orient->axis[1] );
	out[2] = DotProduct( dir, orient->axis[2] );
}

/*
=============
OrientationDirFromWorldDir
=============
*/
void OrientationDirFromWorldDir( const orientation_t *orient, const vec3_t worldDir, vec3_t out ) {
	out[0] = DotProduct( worldDir, orient->axis[0] );
	out[1] = DotProduct( worldDir, orient->axis[1] );
	out[2] = DotProduct( worldDir, orient->axis[2] );
}

