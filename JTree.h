#ifndef __JTree_H__
#define __JTree_H__


struct _TreeNode{
	struct _TreeNode* left;
	struct _TreeNode* right;
	void* val;
	int size;
};

typedef struct _TreeNode JTreen;

struct _Tree{
	JTreen root;
	int len;
};

typedef struct _Tree JTree;

static JTreen* _treen_create(void* val,int size);
JTreen* jtree_init(void* val,int size);

JTreen* jtree_find(JTree* tree,void* val);

void jtree_add(void* val,int size);
void jtree_remove(void* val);

//遍历查找
void jtree_prefind(JTreen* node);
void jtree_midfind(JTreen* node);
void jtree_endfind(JTreen* node);

void jtree_empty(JTreen* node);
void jtree_release(JTreen* node);
#define jtree_auto(...) jtree_autoFree((JTreen*[]){__VA_ARGS__},sizeof((JTreen*[]){__VA_ARGS__})/sizeof(JTreen*))
void jtree_autoFree(JTreen** alls,int size);

#endif
