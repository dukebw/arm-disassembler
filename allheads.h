#ifndef ALLHEADS_H
#define ALLHEADS_H

#include "stopif.h"
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define internal static
#define local_persist static
#define global_variable static

#define ArrayLength(array) sizeof(array)/sizeof((array)[0])

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;

#endif /* ALLHEADS_H */
