#ifndef __JTree_H__
#define __JTree_H__
#include "Any.h"

struct _TreeNode{
	struct _TreeNode* left;
	struct _TreeNode* right;
	Any* any;
};

typedef struct _TreeNode JTreen;

struct _Tree{
	JTreen* root;
};

typedef struct _Tree JTree;

JTree* jtree_init(Any anys[],int len);

JTreen* jtreen_init(void* val,int size);

JTreen* jtreen_find(JTree* tree,void* val,int size);
Any* jtree_find(JTree* tree,void* val,int size);

void jtree_add(JTree* tree,void* val,int size);
void jtree_set(JTree* tree,void* old_val,int old_size,void* new_val,int new_size);
void jtree_remove(JTree* tree,void* val,int size);

//遍历查找
void jtree_prefind(JTreen* node);
void jtree_midfind(JTreen* node);
void jtree_endfind(JTreen* node);

void jtree_release(JTree* tree);
//#define jtreen_auto(...) jtree_autoFree((JTreen*[]){__VA_ARGS__},sizeof((JTreen*[]){__VA_ARGS__})/sizeof(JTreen*))
//void jtreen_autoFree(JTreen** alls,int size);

void jtree_test();
#endif
