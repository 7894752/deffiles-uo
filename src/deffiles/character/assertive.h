/* Assertion reporting shared by the United Offensive character converter. */
#ifndef CHARACTER_ASSERTIVE_H
#define CHARACTER_ASSERTIVE_H

extern void ( *Assert_MessageHook )( const char *message );
int Assert_DoStackTrace( char *buffer, int maxFrames, int skipFrames );
extern "C" int Assert_MyHandler( const char *expression, const char *file,
	int line, const char *format, ... );

#endif
