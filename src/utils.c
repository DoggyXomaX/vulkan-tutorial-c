#include "utils.h"

char *GetExePath( const char* path, uint32_t *size ) {
    uint32_t pathLength = strlen( path );
    char *buffer;
    while( pathLength-- ) {
        if ( path[ pathLength ] == '/' || path[ pathLength ] == '\\' ) {
            buffer = malloc( pathLength + 1 );
            if ( size != NULL ) *size = pathLength + 1;
            buffer[ pathLength ] = '\0';
            strncpy( buffer, path, pathLength );
            return buffer;
        }
    }

    return NULL;
}

char *GetRelativePath( const char* path, const char *filename, uint32_t *size ) {
    if ( path == NULL || filename == NULL ) return NULL;
    uint32_t exePathLength = 0;
    uint32_t filenameLength = strlen( filename );
    char *exePath = GetExePath( path, &exePathLength );

    exePath = realloc( exePath, exePathLength + filenameLength + 1 );
    strcat( exePath, "/" );
    strcat( exePath, filename );

    if ( size != NULL ) *size = exePathLength + filenameLength + 1;
    return exePath;
}
