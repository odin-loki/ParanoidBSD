#include <limits.h>
#include <stdint.h>
#include <sys/types.h>
typedef long long quad_t;
typedef unsigned long long u_quad_t;
#ifndef u_long
typedef unsigned long u_long;
#endif
#ifndef __predict_false
#define __predict_false(exp) __builtin_expect((exp) != 0, 0)
#endif
#ifndef _QUAD_HIGHWORD
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define _QUAD_HIGHWORD 0
#define _QUAD_LOWWORD 1
#else
#define _QUAD_HIGHWORD 1
#define _QUAD_LOWWORD 0
#endif
#endif
