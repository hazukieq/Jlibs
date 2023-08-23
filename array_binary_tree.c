#include <stdio.h>
#include <malloc.h>
#include <memory.h>

struct binaryTree{
	int size;
	char* arr[];
};

typedef struct binaryTree binaryTree;

binaryTree* btree_new(int size){
	binaryTree* btree=malloc(sizeof(binaryTree)+size*sizeof(char*));
	if(btree==NULL) return NULL;
	btree->size=size;
	return btree;
}

int btree_size(binaryTree *btree){
	if(btree==NULL) return 0;
	return btree->size;
}

void btree_set(binaryTree* btree,int index,char* val){
	if(btree==NULL) return;
	if(val==NULL) return;
	
	btree->arr[index]=malloc(strlen(val)+1);
	if(btree->arr[index]==NULL) return;
	memcpy(btree->arr[index],
			val,
			strlen(val)+1);
}

void* btree_get(binaryTree *btree,int index){
	if(btree==NULL) return NULL;
	if(index<0||index>=btree->size) return NULL;
	return btree->arr[index];
}

static int _btree_left(int i){
	return 2*i+1;
}

static int _btree_right(int i){
	return 2*i+2;
}

static int _btree_parent(int i){
	return (i-1)/2;
}

/*
 * 记得手动释放内存
 */
char** btree_levelorder(binaryTree *btree){
	if(btree==NULL) return NULL;

	char** res=malloc(btree->size*sizeof(char*));
	if(res==NULL) return NULL;

	for(int i=0;i<btree->size;i++){
		if(btree->arr[i]!=NULL) 
			res[i]=btree->arr[i];
	}
	return res;
}

#define PRE_ORDER (1<<0)
#define IN_ORDER (1<<1)
#define POST_ORDER (1<<2)
void btree_dfs(binaryTree* btree,int i,char order,char** res){
	if(btree==NULL||res==NULL) return;
	if(btree_get(btree,i)==NULL) return;

	static int count=0;
	
	if(order==PRE_ORDER) 
		res[count++]=btree_get(btree,i);	
	btree_dfs(btree,_btree_left(i),order,res);

	if(order==IN_ORDER) 
		res[count++]=btree_get(btree,i);
	btree_dfs(btree,_btree_right(i),order,res);

	if(order==POST_ORDER)
		res[count++]=btree_get(btree,i);

	if(count==btree->size) count=0;
}

/*
 * 记得手动释放内存
 */
char** btree_preorder(binaryTree* btree){
	char** res=malloc(btree->size*sizeof(char*));
	if(res==NULL) return NULL;
	
	btree_dfs(btree,0,PRE_ORDER,res);
	return res;
}

/*
 * 记得手动释放内存
 */
char** btree_inorder(binaryTree* btree){
	char** res=malloc(btree->size*sizeof(char*));
	if(res==NULL) return NULL;
	
	btree_dfs(btree,0,IN_ORDER,res);
	return res;
}

/*
 * 记得手动释放内存
 */
char** btree_postorder(binaryTree* btree){
	char** res=malloc(btree->size*sizeof(char*));
	if(res==NULL) return NULL;
	
	btree_dfs(btree,0,POST_ORDER,res);
	return res;
}

void btree_free(binaryTree* btree){
	if(btree==NULL) return;
	for(int i=0;i<btree->size;i++){
		if(btree->arr[i]) 
			free(btree->arr[i]);
	}
	free(btree);
}


int main(void){
	binaryTree* btree=btree_new(100);
	for(int i=0;i<100;i++){
		char s[5];
		sprintf(s,"%d",i);
		btree_set(btree,i,s);
	}

	char** res=btree_levelorder(btree);
	//char** res=btree_preorder(btree);
	//char** res=btree_inorder(btree);
	//char** res=btree_postorder(btree);
	for(int j=0;j<100;j++)
		printf("arr: %s\n",res[j]);
	
	free(res);
	btree_free(btree);
	return 0;
}

