#include<stdio.h>
#include<stdlib.h>

//void preorder(struct tree*p)
//{
//    if(p==NULL)return;
//    printf("%s\n", p->file.name);
//    preorder(p->child);
//}
struct tree* search(struct tree* root,int data)
{
    if(root==NULL)
        return;
    if(data==root->data)
        return root;
    struct tree* t = search(root->child,data);
    if(t==NULL)
         t = search(root->sibling,data);
    return t;

}

struct tree* createNode(int data)
{
    struct tree* newnode= (struct tree*)malloc(sizeof(struct tree));
    newnode->child=NULL;
    newnode->sibling=NULL;
    newnode->data=data;
    return newnode;
}

struct tree* createnary(struct tree* root,int data[])
{

    //check if node exist already

    struct tree * newnode = search(root,data[0]);
    //if node does not exist
    if(newnode==NULL)
    {
        newnode= createNode(data[0]);
    }

    struct tree* parent=newnode;
    /////now create node of its children
    int j;
    for(j=0;j<data[1];j++)
    {
        //for first child
        if(j==0)
        {
             parent->child=createNode(data[j+2]);
             parent = parent->child;
        }
        //for all other childs
        else
        {
            parent->sibling=createNode(data[j+2]);
            parent = parent->sibling;
        }

    }

    if(root==NULL)
            root = newnode;
    return root;

}