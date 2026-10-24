/* United Offensive assertion reporting and debugger breakpoint. */
#ifndef UO_ASSERT_H
#define UO_ASSERT_H

extern "C" int MyAssertHandler( const char *expression, const char *file,
    int line, ...);

#define assert( exp ) \
    do { \
        static int s_ignore = 0; \
        if ( !s_ignore && !( exp ) ) { \
            int result = MyAssertHandler( #exp, __FILE__, __LINE__, 0 ); \
            if ( result < 0 ) { \
                s_ignore = 1; \
            } else if ( result > 0 ) { \
                __asm { int 3 } \
            } \
        } \
    } while ( 0 )

#endif
