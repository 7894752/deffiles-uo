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

#include "q_parse.h"
#include "q_assert.h"

typedef struct parseInfo_t {
	char token[MAX_TOKEN_CHARS];
	int currentLine;
	qboolean ungetReady;
	qboolean spaceDelimited;
	qboolean csv;
	qboolean parseNegativeNumbers;
	int ungetLineSave;
	const char *ungetTokenSave;
	char filename[64];
} parseInfo_t;

static const char *com_parseSeparators[] = {
	"+=", "-=", "*=", "/=", "&=", "|=",
	"++", "--", "&&", "||", "<=", ">=", "==", "!=",
	NULL
};

static parseInfo_t parseInfo_stack[MAX_PARSE_SESSIONS] = {
	{ "", 1, qfalse, qtrue, qfalse, qtrue, 1, NULL, "" }
};
static parseInfo_t *parseInfo = parseInfo_stack;
static int parseInfo_level;
static char com_lineBuffer[MAX_TOKEN_CHARS] = {0};
static const char *prevTokenPos = NULL;
static const char *lastTokenPos = NULL;


/*
===================
Com_BeginParseSession
===================
*/
void Com_BeginParseSession( const char *name ) {
	if ( parseInfo_level == MAX_PARSE_SESSIONS - 1 ) {
		Com_Error( ERR_FATAL, "\x15" "Com_BeginParseSession: session overflow" );
	}
	parseInfo_level++;
	parseInfo = &parseInfo_stack[parseInfo_level];
	parseInfo->currentLine = 1;
	parseInfo->ungetReady = qfalse;
	parseInfo->spaceDelimited = qtrue;
	parseInfo->csv = qfalse;
	Q_strncpyz( parseInfo->filename, name, sizeof( parseInfo->filename ) );
}

/*
===================
Com_EndParseSession
===================
*/
void Com_EndParseSession( void ) {
	if ( !parseInfo_level ) {
		Com_Error( ERR_FATAL, "\x15" "Com_EndParseSession: session underflow" );
	}
	parseInfo_level--;
	parseInfo = &parseInfo_stack[parseInfo_level];
}

/*
=============
Com_ResetParseSessions
=============
*/
void Com_ResetParseSessions( void ) {
	parseInfo_level = 0;
	parseInfo = &parseInfo_stack[parseInfo_level];
}

/*
=============
Com_SetSpaceDelimited
=============
*/
void Com_SetSpaceDelimited( qboolean enabled ) {
	parseInfo->spaceDelimited = enabled;
}

/*
=============
Com_SetCSV
=============
*/
void Com_SetCSV( qboolean enabled ) {
	parseInfo->csv = enabled;
}

/*
=============
Com_SetParseNegativeNumbers
=============
*/
void Com_SetParseNegativeNumbers( qboolean enabled ) {
	parseInfo->parseNegativeNumbers = enabled;
}

/*
===================
Com_GetCurrentParseLine
===================
*/
int Com_GetCurrentParseLine( void ) {
	return parseInfo->currentLine;
}

/*
===================
Com_ScriptError

Prints the script name and line number in the message
===================
*/
void Com_ScriptError( const char *fmt, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );
	Com_Error( ERR_DROP, "\x15" "File %s, line %i: %s", parseInfo->filename, parseInfo->currentLine, string );
}

/*
=============
Com_ScriptWarning
=============
*/
void Com_ScriptWarning( const char *fmt, ... ) {
	va_list argptr;
	char string[32000];

	va_start( argptr, fmt );
	vsprintf( string, fmt, argptr );
	va_end( argptr );

	Com_Printf( "File %s, line %i: %s", parseInfo->filename, parseInfo->currentLine, string );
}

/*
===================
Com_UngetToken

Calling this will make the next Com_Parse return
the current token instead of advancing the pointer
===================
*/
void Com_UngetToken( void ) {
	if ( parseInfo->ungetReady ) {
		Com_ScriptError( "UngetToken called twice" );
	}
	parseInfo->ungetReady = qtrue;
	lastTokenPos = prevTokenPos;
}

/*
=============
Com_ParseSetMark
=============
*/
void Com_ParseSetMark( const char **text, parseMark_t *mark ) {
	IW_ASSERT( text );
	IW_ASSERT( mark );

	mark->currentLine = parseInfo->currentLine;
	mark->data = *text;

	mark->ungetReady = parseInfo->ungetReady;
	mark->ungetLineSave = parseInfo->ungetLineSave;
	mark->ungetTokenSave = parseInfo->ungetTokenSave;
}

/*
=============
Com_ParseReturnToMark
=============
*/
void Com_ParseReturnToMark( const char **text, const parseMark_t *mark ) {
	IW_ASSERT( text );
	IW_ASSERT( mark );
	parseInfo->currentLine = mark->currentLine;
	*text = mark->data;
	parseInfo->ungetReady = mark->ungetReady;
	parseInfo->ungetLineSave = mark->ungetLineSave;
	parseInfo->ungetTokenSave = mark->ungetTokenSave;
}

/*
=============
Com_Compress
=============
*/
int Com_Compress( char *data_p ) {
	char *datai, *datao;
	char c;
	int size;
	qboolean whitespace = qfalse;

	size = 0;
	datai = datao = data_p;
	if ( datai ) {
		while ( ( c = *datai ) != 0 ) {
			if ( c == '\r' || c == '\n' ) {
				*datao = c;
				datao++;
				whitespace = qfalse;
				datai++;
				size++;
			} else if ( c == '/' && datai[1] == '/' ) {
				while ( *datai && *datai != '\n' ) datai++;
				whitespace = qfalse;
			} else if ( c == '/' && datai[1] == '*' ) {
				while ( *datai && ( *datai != '*' || datai[1] != '/' ) ) datai++;
				if ( *datai ) datai += 2;
				whitespace = qfalse;
			} else {
				if ( whitespace ) {
					*datao = ' ';
					datao++;
				}
				*datao = c;
				datao++;
				datai++;
				whitespace = qfalse;
				size++;
			}
		}
	}
	*datao = 0;
	return size;
}

/*
=============
Com_GetLastTokenPos
=============
*/
const char *Com_GetLastTokenPos( void ) {
	return lastTokenPos;
}

static char *Com_ParseExt( const char **data_p, qboolean allowLineBreaks );
static const char *SkipWhitespace( const char *data, qboolean *hasNewLines );
char *Com_ParseCSV( const char **data_p, qboolean allowLineBreaks );

/*
===================
Com_Parse
===================
*/
char *Com_Parse( const char **data_p ) {
	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	return Com_ParseExt( data_p, qtrue );
}

/*
==============
Com_ParseExt

Parse a token out of a string. Return an empty string at end of input,
or at a line break when allowLineBreaks is qfalse.
==============
*/
static char *Com_ParseExt( const char **data_p, qboolean allowLineBreaks ) {
	char c = 0;
	int len;
	qboolean hasNewLines = qfalse;
	const char *data;

	if ( !data_p ) {
		Com_Error( ERR_FATAL, "\x15" "Com_ParseExt: NULL data_p" );
	}
	data = *data_p;
	len = 0;
	parseInfo->token[0] = 0;

	if ( !data ) {
		*data_p = NULL;
		return parseInfo->token;
	}

	parseInfo->ungetLineSave = parseInfo->currentLine;
	parseInfo->ungetTokenSave = *data_p;

	if ( parseInfo->csv ) {
		return Com_ParseCSV( data_p, allowLineBreaks );
	}

	for ( ;; ) {
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data ) {
			*data_p = NULL;
			return parseInfo->token;
		}
		if ( hasNewLines && !allowLineBreaks ) {
			*data_p = data;
			return parseInfo->token;
		}
		c = *data;
		if ( c == '/' && data[1] == '/' ) {
			while ( *data && *data != '\n' ) {
				data++;
			}
			continue;
		}
		if ( c == '/' && data[1] == '*' ) {
			while ( *data && ( *data != '*' || data[1] != '/' ) ) {
				if ( *data == '\n' ) {
					parseInfo->currentLine++;
				}
				data++;
			}
			if ( *data ) {
				data += 2;
			}
			continue;
		}
		break;
	}

	prevTokenPos = lastTokenPos;
	lastTokenPos = data;

	if ( c == '\"' ) {
		data++;
		for ( ;; ) {
			c = *data++;
			if ( c == '\\' && *data == '\"' ) {
				c = *data++;
			} else if ( c == '\"' || !c ) {
				parseInfo->token[len] = 0;
				*data_p = data;
				return parseInfo->token;
			} else if ( *data == '\n' ) {
				parseInfo->currentLine++;
			}
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
		}
	}

	if ( parseInfo->spaceDelimited ) {
		do {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( c > ' ' );

		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		parseInfo->token[len] = 0;
		*data_p = data;
		return parseInfo->token;
	}

	if ( ( c >= '0' && c <= '9' ) ||
		 ( parseInfo->parseNegativeNumbers && c == '-' && data[1] >= '0' && data[1] <= '9' ) ||
		 ( c == '.' && data[1] >= '0' && data[1] <= '9' ) ) {
		do {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( ( c >= '0' && c <= '9' ) || c == '.' );

		if ( c == 'e' || c == 'E' ) {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
			if ( c == '-' || c == '+' ) {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					parseInfo->token[len] = c;
					len++;
				}
				data++;
				c = *data;
			}
			do {
				if ( len < MAX_TOKEN_CHARS - 1 ) {
					parseInfo->token[len] = c;
					len++;
				}
				data++;
				c = *data;
			} while ( c >= '0' && c <= '9' );
		}
		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		parseInfo->token[len] = 0;
		*data_p = data;
		return parseInfo->token;
	}
	if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
				c == '_' || c == '/' || c == '\\' ) {
		do {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = c;
				len++;
			}
			data++;
			c = *data;
		} while ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) ||
				  c == '_' || ( c >= '0' && c <= '9' ) ||
				  c == '/' || c == '\\' || c == ':' || c == '.' );
		if ( len == MAX_TOKEN_CHARS ) {
			len = 0;
		}
		parseInfo->token[len] = 0;
		*data_p = data;
		return parseInfo->token;
	}
	{
		const char **sep = com_parseSeparators;
		for ( ; *sep; sep++ ) {
			int seplen = strlen( *sep );
			int i;
			for ( i = 0; i < seplen; i++ ) {
				if ( data[i] != ( *sep )[i] ) {
					break;
				}
			}
			if ( i == seplen ) {
				memcpy( parseInfo->token, *sep, seplen );
				parseInfo->token[seplen] = 0;
				data += seplen;
				*data_p = data;
				return parseInfo->token;
			}
		}
		parseInfo->token[0] = *data;
		parseInfo->token[1] = 0;
		data++;
		*data_p = data;
		return parseInfo->token;
	}


}

/*
==============
SkipWhitespace

Skip spaces and count line breaks. Return NULL at the end of the input.
==============
*/
static const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
	int c;
	while ( ( c = *data ) <= ' ' ) {
		if ( !c ) return NULL;
		if ( c == '\n' ) {
			parseInfo->currentLine++;
			*hasNewLines = qtrue;
		}
		data++;
	}
	return data;
}

/*
=============
Com_ParseCSV
=============
*/
static char *Com_ParseCSV( const char **data_p, qboolean allowLineBreaks ) {
	int len;
	const char *data;

	data = *data_p;
	len = 0;
	parseInfo->token[0] = 0;

	if ( !allowLineBreaks ) {
		if ( *data == '\r' || *data == '\n' ) {
			return parseInfo->token;
		}
	} else {
		while ( *data == '\r' || *data == '\n' ) {
			data++;
		}
	}

	prevTokenPos = lastTokenPos;
	lastTokenPos = data;

	while ( *data && *data != ',' && *data != '\n' ) {
		if ( *data == '\r' ) {
			data++;
			continue;
		}
		if ( *data != '"' ) {
			if ( len < MAX_TOKEN_CHARS - 1 ) {
				parseInfo->token[len] = *data;
				len++;
			}
			data++;
		} else {
			data++;
			for ( ;; ) {
				if ( *data == '"' ) {
					if ( data[1] == '"' ) {
					if ( len < MAX_TOKEN_CHARS - 1 ) {
						parseInfo->token[len] = '"';
						len++;
					}
					data += 2;
					} else {
						data++;
						break;
					}
				} else {
					if ( len < MAX_TOKEN_CHARS - 1 ) {
						parseInfo->token[len] = *data;
						len++;
					}
					data++;
				}
			}
		}
	}

	if ( *data ) {
		if ( *data != '\n' ) {
			data++;
		}
		*data_p = data;
	} else {
		*data_p = NULL;
	}

	parseInfo->token[len] = 0;
	return parseInfo->token;
}

/*
===================
Com_ParseOnLine
===================
*/
char *Com_ParseOnLine( const char **data_p ) {
	if ( parseInfo->ungetReady ) {
		parseInfo->ungetReady = qfalse;
		if ( !parseInfo->spaceDelimited ) {
			return parseInfo->token;
		}
		*data_p = parseInfo->ungetTokenSave;
		parseInfo->currentLine = parseInfo->ungetLineSave;
	}
	return Com_ParseExt( data_p, qfalse );
}

/*
==================
Com_MatchToken
==================
*/
void Com_MatchToken( const char **data_p, const char *match, int warn ) {
	char    *token;

	token = Com_Parse( data_p );

	if ( strcmp( token, match ) ) {
		if ( warn ) {
			Com_ScriptWarning( "MatchToken: %s != %s", token, match );
		} else {
			Com_ScriptError( "MatchToken: %s != %s", token, match );
		}
	}
}

/*
=================
Com_SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
int Com_SkipBracedSection( const char **data_p, int startDepth ) {
	char *token;
	int depth;
	int found;
	found = 0;
	depth = 0;

	do {
		token = Com_Parse( data_p );
		if ( !token[1] ) {
			if ( token[0] == '{' ) {
				if ( depth == startDepth ) found = 1;
				else depth++;
			} else if ( token[0] == '}' ) depth--;
		}
	} while ( depth && *data_p );
	return found;
}

/*
=================
Com_SkipRestOfLine
=================
*/
void Com_SkipRestOfLine( const char **data_p ) {
    const char *p;
    int c;

    p = *data_p;
    if ( !p ) {
        return;
    }
    while ( ( c = *p ) != 0 ) {
        p++;
        if ( c == '\n' ) {
            parseInfo->currentLine++;
            break;
        }
    }
    *data_p = p;
}

/*
====================
Com_ParseRestOfLine
====================
*/
char *Com_ParseRestOfLine( const char **data_p ) {
	char    *token;

	com_lineBuffer[0] = 0;
	for ( ;; ) {
		token = Com_ParseOnLine( data_p );
		if ( !token[0] ) {
			break;
		}

		if ( com_lineBuffer[0] ) {
			Q_strcat( com_lineBuffer, sizeof( com_lineBuffer ), " " );
		}
		Q_strcat( com_lineBuffer, sizeof( com_lineBuffer ), token );
	}
	return com_lineBuffer;
}

/*
=============
Com_ParseFloat
=============
*/
float Com_ParseFloat( const char **data_p ) {
	char    *token;

	token = Com_Parse( data_p );
	if ( !*token ) {
		return 0;
	}
	return atof( token );
}

/*
=============
Com_ParseInt
=============
*/
int Com_ParseInt( const char **data_p ) {
	char    *token;

	token = Com_Parse( data_p );
	if ( !*token ) {
		return 0;
	}
	return atoi( token );
}

/*
=============
Com_Parse1DMatrix
=============
*/
void Com_Parse1DMatrix( const char **data_p, int x, float *m ) {
	char	*token;
	int		i;

	Com_MatchToken( data_p, "(", 0 );

	for (i = 0 ; i < x ; i++) {
		token = Com_Parse(data_p);
		m[i] = atof(token);
	}

	Com_MatchToken( data_p, ")", 0 );
}

/*
=============
Com_Parse2DMatrix
=============
*/
void Com_Parse2DMatrix( const char **data_p, int y, int x, float *m ) {
	int		i;

	Com_MatchToken( data_p, "(", 0 );

	for (i = 0 ; i < y ; i++) {
		Com_Parse1DMatrix (data_p, x, m + i * x);
	}

	Com_MatchToken( data_p, ")", 0 );
}

/*
=============
Com_Parse3DMatrix
=============
*/
void Com_Parse3DMatrix( const char **data_p, int z, int y, int x, float *m ) {
	int		i;

	Com_MatchToken( data_p, "(", 0 );

	for (i = 0 ; i < z ; i++) {
		Com_Parse2DMatrix (data_p, y, x, m + i * x*y);
	}

	Com_MatchToken( data_p, ")", 0 );
}
