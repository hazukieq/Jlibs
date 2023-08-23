#include "JTree.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "JQueue.h"

static JTreen* _treen_create(void* val,int size){
	if(val==NULL||size==0) return NULL;

	JTreen* node=malloc(sizeof(JTreen));
	if(node==NULL) return NULL;

	node->val=malloc(size);
	if(node->val==NULL){
		free(node);
		return NULL;
	}

	memcpy(node->val,val,size);
	node->size=size;
	node->left=NULL;
	node->right=NULL;
	return node;
}

JTreen* jtree_init(void* val,int size){
	return _treen_create(val,size);
}

//前序遍历 根->左->右
void jtree_prefind(JTreen* node){
	if(node){
		printf("find: %s\n",(char*)node->val);
		jtree_prefind(node->left);
		jtree_prefind(node->right);
	}
}

//中序遍历 左->根->右
void jtree_midfind(JTreen *node){
	if(node){
		jtree_midfind(node->left);
		printf("find: %s\n",(char*)node->val);
		jtree_midfind(node->right);
	}
}
//后序遍历 左->右->根
void jtree_endfind(JTreen *node){
	if(node){
		jtree_endfind(node->left);
		jtree_endfind(node->right);
		printf("find: %s\n",(char*)node->val);
	}
}


void jtree_release(JTreen* node){
	if(node==NULL) return;
	
	if(node->left) jtree_release(node->left);
	if(node->right) jtree_release(node->right);
	
	if(node->val) free(node->val);
	free(node);
}

void jtree_bfs(JTreen* node){
	Jque* jq=jque_init();
	jque_push(jq,node,sizeof(JTreen));

	while(jq->len){
		JTreen* jcur=(JTreen*)jque_pop(jq);
		printf("nptr:%p,ptr: %p,%s\n",node,jcur,(char*)jcur->val);
	
		if(jcur->left) jque_push(jq,jcur->left,sizeof(JTreen));
		if(jcur->right) jque_push(jq,jcur->right,sizeof(JTreen));
		jque_pop_free(jcur);
	}
	jque_release(jq);
}


int main(void){
	JTreen* node=jtree_init("hello",6);
	JTreen* left=jtree_init("world",6);
	JTreen* right=jtree_init("welcome",8);
	
	node->left=left; 
	node->right=right;

	jtree_bfs(node);
	jtree_release(node);
	
	return 0;
}

