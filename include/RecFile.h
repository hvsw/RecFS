#include <stdio.h>
#include <stdlib.h>

#ifndef INCLUDE_RECFILE

#define INCLUDE_RECFILE

#define RECFILE_TYPE_FILE 1
#define RECFILE_TYPE_DIRECTORY 2

struct RecFile {
    char name[32];
    
    // Valid values:
    // RECFILE_TYPE_FILE, RECFILE_TYPE_DIRECTORY
    int type;
    
    int startingBlock;
    int size;
    DWORD pointer;
};

typedef struct RecFile RecFile;

#endif
