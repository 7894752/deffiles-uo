/* Parser sessions and caller-owned cursor marks. */
#ifndef UO_XMODEL_Q_PARSE_H
#define UO_XMODEL_Q_PARSE_H

#include "q_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TOKEN_CHARS 1024
#define MAX_PARSE_SESSIONS 16

typedef struct parseMark_t {
	int currentLine;
	const char *data;
	qboolean ungetReady;
	int ungetLineSave;
	const char *ungetTokenSave;
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
void Com_ParseSetMark( const char **text, parseMark_t *mark );
void Com_ParseReturnToMark( const char **text, const parseMark_t *mark );
int Com_Compress( char *data_p );
const char *Com_GetLastTokenPos( void );
char *Com_Parse( const char **data_p );
char *Com_ParseOnLine( const char **data_p );
void Com_MatchToken( const char **data_p, const char *match, int warn );
int Com_SkipBracedSection( const char **data_p, int startDepth );
void Com_SkipRestOfLine( const char **data_p );
char *Com_ParseRestOfLine( const char **data_p );
float Com_ParseFloat( const char **data_p );
int Com_ParseInt( const char **data_p );
void Com_Parse1DMatrix( const char **data_p, int x, float *m );
void Com_Parse2DMatrix( const char **data_p, int y, int x, float *m );
void Com_Parse3DMatrix( const char **data_p, int z, int y, int x, float *m );

#ifdef __cplusplus
}
#endif

#endif
