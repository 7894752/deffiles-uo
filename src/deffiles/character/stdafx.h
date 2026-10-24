/*
===========================================================================
Call of Duty: United Offensive converter plugins (2004) - source reconstruction.

Common includes and assertion support for the character converter.

Reconstructed to compile to the original binary. Distributed under the terms in
COPYING (GPL version 3 or later, with id Software's additional terms).
===========================================================================
*/

#ifndef STDAFX_H
#define STDAFX_H

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "../convert_value.h"

#define ARRAY_COUNT( array ) ( sizeof( array ) / sizeof( (array)[0] ) )
// Shared scratch space for formatted script and error messages.
#define MAX_VA_STRING 32000

#include "assertive.h"

#define assert( exp ) \
	do { \
		static int s_ignore; \
		if ( !s_ignore && !( exp ) ) { \
			int result = Assert_MyHandler( #exp, __FILE__, __LINE__, 0 ); \
			if ( result < 0 ) { \
				s_ignore = 1; \
			} else if ( result > 0 ) { \
				__asm { int 3 } \
			} \
		} \
	} while ( 0 )

#endif
