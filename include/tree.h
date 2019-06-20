#include<stdio.h>
#include<stdlib.h>
#include"RecFile.h"

struct tree{
    struct RecFile file;
    struct tree *children ;
};

void preorder(struct tree*p);

struct tree* search(struct tree* root,int data);

struct tree* createNode(int data);

struct tree* createnary(struct tree* root,int data[]);