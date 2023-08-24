#include "JTree.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "JDict.h"
#include "JQueue.h"
#include "JSha256.h"

#define _S(arg) sizeof(arg)

struct Any{
	void* val;
	int size;
};
typedef struct Any Any;

static unsigned int _has_fn(void* val,int size){
	unsigned char* hash;
	jsha_hash(val,size,&hash);
	
	unsigned int hash_int=*((unsigned int*)hash);
	printf("hash_int:%d\n",hash_int);
	
	if(hash) free(hash);
	
	return hash_int;
}

static int _compare(const void* a,const void* b){
	return (*(int*)a-*(int*)b);
}

static void _sortArr(Any arrs[],int len,int hashes[],int hlen){
	if(hlen!=len) return;
	for(int i=0;i<len;i++)
		hashes[i]=(int)_has_fn((arrs[i]).val,(arrs[i]).size);

	for(int j=0;j<hlen;j++)
		printf("val: %d\n",hashes[j]);
}

static JTreen* _treen_create(void* val,int size){
	if(val==NULL||size==0) return NULL;

	JTreen* node=malloc(_S(JTreen));
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
	jque_push(jq,node,_S(JTreen));

	while(jq->len){
		JTreen* jcur=(JTreen*)jque_pop(jq);
		printf("jcur: %s\n",(char*)jcur->val);
		if(jcur->left)
			jque_push(jq,jcur->left,_S(JTreen));
		if(jcur->right) 
			jque_push(jq,jcur->right,_S(JTreen));
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
	
	unsigned int s=_has_fn("welcome",8);
	printf("%d %% %d=%d\n",s,100,s%100);
	
	Any anys[3]={
		{"hello",6},{"world",6},{"welcome",8}
	};
	int any_hashes[3];
	_sortArr(anys,3,any_hashes,3);
	
	return 0;
}

