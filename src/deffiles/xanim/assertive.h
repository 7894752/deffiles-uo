/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#ifndef ASSERTIVE_H
#define ASSERTIVE_H

extern void ( *AssertCallback )( const char *message );

extern "C" int MyAssertHandler( const char *expression, const char *file, int line , ...);

#define IW_ASSERT( expression ) \
	{ \
		static char s_bIgnoreAssert = 0; \
		if ( !( expression ) && !s_bIgnoreAssert ) { \
			switch ( MyAssertHandler( #expression, __FILE__, __LINE__ ) ) { \
			case 2: s_bIgnoreAssert = 1; break; \
			case 0: __asm { int 3 } break; \
			} \
		} \
	}

#endif
