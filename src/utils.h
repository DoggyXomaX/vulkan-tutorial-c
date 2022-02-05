#ifndef __UTILS_H__
#define __UTILS_H__

#ifndef min
    #define min( a, b ) ( (a) < (b) ? (a) : (b) )
#endif

#ifndef max
    #define max( a, b ) ( (a) > (b) ? (a) : (b) )
#endif

#ifndef clamp
    #define clamp( value, min_value, max_value ) ( min( max( value, min_value ), max_value ) )
#endif

#endif