/* Model, surface and skin writer interfaces. */
#ifndef UO_XMODEL_H
#define UO_XMODEL_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <malloc.h>
#include <limits.h>
#include "../convert_value.h"
#include "xmodel_model.h"
#include "script.h"

#pragma intrinsic(memset, memcpy)
#include "q_endian.h"

#define VectorCopy( a, b ) ( (b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2] )
#define VectorScale( v, s, o ) ( (o)[0] = (v)[0] * (s), (o)[1] = (v)[1] * (s), (o)[2] = (v)[2] * (s) )

extern "C" float VectorNormalize4D( float q[4] );
extern loadcollmodel_t collmodel;
void ConvertMatToQuat( float m[3][3], float quat[4] );

#endif
