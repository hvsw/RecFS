#include <stdio.h>
#include <stdlib.h>

#ifndef INCLUDE_TREE

#include "RecFile.h"
#define INCLUDE_TREE

struct tree {
    struct RecFile file;
    struct tree *children ;
};

typedef struct tree tree;

#endif

//struct tree* search(struct tree* root, RecFile data);
//struct tree* createNode(int data);
//struct tree* createnary(struct tree* root,int data[]);
