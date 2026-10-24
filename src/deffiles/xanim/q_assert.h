/* Runtime assertions shared by C converter support code. */
#ifndef UO_XANIM_Q_ASSERT_H
#define UO_XANIM_Q_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif
int MyAssertHandler(const char *, const char *, int, ...);
#ifdef __cplusplus
}
#endif
#define Q_ASSERT(expression, message) \
    { \
        static char s_bIgnoreAssert = 0; \
        if (!(expression) && !s_bIgnoreAssert) { \
            switch (MyAssertHandler((message), __FILE__, __LINE__)) { \
            case 2: s_bIgnoreAssert = 1; break; \
            case 0: __asm { int 3 } break; \
            } \
        } \
    }
#define IW_ASSERT(expression) Q_ASSERT(expression, #expression)

#endif
