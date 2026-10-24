/* Parser sessions and caller-owned cursor marks. */
#ifndef UO_XANIM_Q_PARSE_H
#define UO_XANIM_Q_PARSE_H

#include "q_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TOKEN_CHARS 1024
#define MAX_PARSE_SESSIONS 16

typedef struct parseMark_t {
	int currentLine;
	char *data;
	qboolean ungetReady;
	int ungetLineSave;
	char *ungetTokenSave;
} parseMark_t;

void Com_BeginParseSession( const char *name );
void Com_EndParseSession( void );
void Com_ResetParseSessions( void );
void Com_SetSpaceDelimited( qboolean enabled );
void Com_SetCSV( qboolean enabled );
void Com_SetParseNegativeNumbers( qboolean enabled );
int Com_GetCurrentParseLine( void );
void Com_ScriptError( const char *fmt, ... );
void Com_ScriptWarning( const char *fmt, ... );
void Com_UngetToken( void );
void Com_ParseSetMark( char **text, parseMark_t *mark );
void Com_ParseReturnToMark( char **text, const parseMark_t *mark );
int Com_Compress( char *data_p );
char *Com_GetLastTokenPos( void );
char *Com_Parse( char **data_p );
char *Com_ParseOnLine( char **data_p );
void Com_MatchToken( char **data_p, const char *match, int warn );
int Com_SkipBracedSection( char **data_p, int startDepth );
void Com_SkipRestOfLine( char **data_p );
char *Com_ParseRestOfLine( char **data_p );
float Com_ParseFloat( char **data_p );
int Com_ParseInt( char **data_p );
void Com_Parse1DMatrix( char **data_p, int x, float *m );
void Com_Parse2DMatrix( char **data_p, int y, int x, float *m );
void Com_Parse3DMatrix( char **data_p, int z, int y, int x, float *m );

#ifdef __cplusplus
}
#endif

#endif
