#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef min
    #define min( a, b ) ( (a) < (b) ? (a) : (b) )
#endif

#ifndef max
    #define max( a, b ) ( (a) > (b) ? (a) : (b) )
#endif

#ifndef clamp
    #define clamp( value, min_value, max_value ) ( min( max( value, min_value ), max_value ) )
#endif

#define UTILS_MAX_PATH_SIZE 512

char *GetExePath( const char*, uint32_t* );
char *GetRelativePath( const char*, const char*, uint32_t* );

#endif