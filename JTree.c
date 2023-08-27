#include "JTree.h"
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>
#include "Any.h"
#include "JQueue.h"
#include "JSha256.h"

#define _S(arg) sizeof(arg)
#define _hash(node) _hash_fn(anyVal(node ->any),anySize(node->any))

static int _hash_fn(void* val,int size){
	unsigned char* hash;
	jsha_hash(val,size,&hash);
	
	unsigned int hash_int=*((unsigned int*)hash);
	if(hash) free(hash);
	
	return hash_int;
}
//从小到大排序
static int _compare(const void* a,const void* b){
	int num_a=_hash_fn(((Any*)a)->val,((Any*)a)->size);
	int num_b=_hash_fn(((Any*)b)->val,((Any*)b)->size);

	if(num_a<num_b){
		//printf("%d<%d\n",num_a,num_b);
		return -1;//num_a排在num_b之前
	}
	else if(num_a>num_b) {
		//printf("%d>%d\n",num_a,num_b);
		return 1;//num_a排在num_b之后
	}
	else return 0;
}

static void _sort_array(Any anys[],int len){
	if(len==0) return;
	qsort(anys,len,_S(Any),_compare);
}

static JTreen* _treen_create(void* val,int size){
	if(val==NULL||size==0) return NULL;

	JTreen* node=malloc(_S(JTreen));
	if(node==NULL) return NULL;

	Any* any=anyOf(val,size);
	if(any==NULL){
		free(node);
		return NULL;
	}
	
	node->any=any;
	node->left=NULL;
	node->right=NULL;
	return node;
}

static JTreen* _build_treen(Any anys[],int start,int end){
	if(start>end) return NULL;
	int mid=(start+end)/2;
	
	JTreen* root=_treen_create(anys[mid].val,anys[mid].size);
	
	root->left=_build_treen(anys,start,mid-1);
	root->right=_build_treen(anys,mid+1,end);
	return root;
}

static JTree* _tree_init(Any anys[],int len){
	JTree* tree=malloc(_S(JTree));

	int hash_arr[len];
	_sort_array(anys,len);
	
	JTreen* root=_build_treen(anys,0,len-1);
	tree->root=root;
	return tree;
}


static JTreen* _treen_find(JTree* tree,void* val,int len){
	int num=_hash_fn(val,len);
	JTreen* cur=tree->root;
	while(cur!=NULL){
		if(_hash(cur)>num) cur=cur->left;
		else if(_hash(cur)<num) cur=cur->right;
		else break;
	}
	return cur;
}

static void _treen_add(JTree* tree,void* val,int size){
	int num=_hash_fn(val,size);
	JTreen* cur=tree->root;
	JTreen* pre=NULL;
	while(cur){
		if(_hash(cur)==num) return;
		
		pre=cur;
		if(_hash(cur)>num) cur=cur->left;
		else cur=cur->right;
	}
	JTreen* node=_treen_create(val,size);
	if(_hash(pre)<num) pre->right=node;
	else pre->left=node;
}

static void _treen_del(JTree* tree,void* val,int size){
	int num=_hash_fn(val,size);
	JTreen* cur=tree->root;
	JTreen* pre=NULL;
	while(cur){
		if(_hash(cur)==num) break;
		
		pre=cur;
		if(_hash(cur)>num) cur=cur->left;
		else cur=cur->right;
	}
	
	if(cur==NULL){	
		printf("cannot find any to delete\n");
		return;
	}
	
	if(cur->left==NULL||cur->right==NULL){
		JTreen* tmp=cur->left?cur->left:cur->right;
		if(pre->left==cur) pre->left=tmp;
		else pre->right=tmp;
		if(cur->any) anyFree(cur->any);
		free(cur);
	}else{
		JTreen* child=cur->left;
		while(child->right)
			child=child->right;
	
		Any* any=anyCopy(child->any);
		_treen_del(tree, any->val,any->size);
		anyFree(cur->any);
		cur->any=any;
	}
}

static void _treen_release(JTreen* node){
	if(node==NULL) return;
	
	if(node->left) _treen_release(node->left);
	if(node->right) _treen_release(node->right);
	
	if(node->any)
		anyFree(node->any);
	free(node);
}




/********* PUBLIC METHODS ************************/

/*JTreen* jtreen_init(void* val,int size){
	return _treen_create(val,size);
}*/

JTree* jtree_init(Any anys[],int len){
	if(anys==NULL||len==0) return NULL;
	return _tree_init(anys,len);
}

JTreen* jtreen_find(JTree* tree,void* val,int size){
	if(tree==NULL||tree->root==NULL||val==NULL) return NULL;
	return _treen_find(tree,val,size); 
}

Any* jtree_find(JTree* tree,void* val,int size){
	if(tree==NULL||tree->root==NULL||val==NULL) return NULL;
	
	JTreen* treen=_treen_find(tree,val,size);
	if(treen) return treen->any;
	return NULL;
}

void jtree_add(JTree* tree,void *val, int size){
	if(tree==NULL||tree->root==NULL||val==NULL) return;
	_treen_add(tree,val,size);
}

void jtree_set(JTree* tree,void* old_val,int old_size,void* new_val,int new_size){
	if(tree==NULL||tree->root==NULL||old_val==NULL||new_val==NULL) return;
	int num=_hash_fn(old_val,old_size);
	//之所以不能直接找到修改的原因是:
	//   - 必须维持二叉搜索树的定义(左<根<右,子树亦如是)
	//   避免二叉搜索树的崩塌
	//删除旧节点
	_treen_del(tree,old_val,old_size);
	//添加新节点
	_treen_add(tree,new_val,new_size);
}

void jtree_remove(JTree* tree,void* val,int size){
	if(tree==NULL||tree->root==NULL||val==NULL) return;
	_treen_del(tree,val,size);
}

//前序遍历 根->左->右
void jtree_prefind(JTreen* node){
	if(node){
		anyLog(node->any,char*,s);
		jtree_prefind(node->left);
		jtree_prefind(node->right);
	}
}

//中序遍历 左->根->右
void jtree_midfind(JTreen *node){
	if(node){
		jtree_midfind(node->left);
		anyLog(node->any,char*,s);
		jtree_midfind(node->right);
	}
}

//后序遍历 左->右->根
void jtree_endfind(JTreen *node){
	if(node){
		jtree_endfind(node->left);
		jtree_endfind(node->right);
		anyLog(node->any,char*,s);
	}
}

//层序遍历,从上往下一层一层遍历
void jtree_bfs(JTreen* node){
	Jque* jq=jque_init();
	jque_push(jq,node,_S(JTreen));

	while(jq->len){
		JTreen* jcur=(JTreen*)jque_pop(jq);
		anyLog(jcur->any,char*,s);
		if(jcur->left)
			jque_push(jq,jcur->left,_S(JTreen));
		if(jcur->right) 
			jque_push(jq,jcur->right,_S(JTreen));
		jque_pop_free(jcur);
	}
	jque_release(jq);
}


void jtree_release(JTree* tree){
	if(tree==NULL) return;
	_treen_release(tree->root);
	free(tree);
}



void jtree_test(){
	/*--ALL PASS--*/
	Any anys[]={
		anyI("hello",6),
		anyI("world",6),
		anyI("welcome",8),
		anyI("goodbye",8),
		anyI("amazing",8),
		anyStr("hahahahaha,this is a function you made!! It is so exquisite!"),
	};
	
	JTree* tree=jtree_init(anys,_S(anys)/_S(Any));
	jtree_set(tree,"hello",6,"aloha",6);
	_treen_add(tree,"find,good!", 11);

	/*--jtree_find--*/
	putchar('\n');
	Any* ch=jtree_find(tree,"find,good!",11);
	if(ch) anyLog(ch,char*,s);	

	/*--PASS jtreen_find--*/
	putchar('\n');	
	JTreen* t=jtreen_find(tree,"aloha",6); 
	if(t) anyLog(t->any,char*,s);
	putchar('\n');
	
	jtree_remove(tree,"world",6);
	
	printf("prefind:\n");
	jtree_prefind(tree->root);	
	putchar('\n');

	printf("midfind:\n");
	jtree_midfind(tree->root);	
	putchar('\n');

	printf("endfind:\n");
	jtree_endfind(tree->root);
	putchar('\n');

	printf("bfs:\n");
	jtree_bfs(tree->root);

	jtree_release(tree);
}

/*int main(void){
	jtree_test();
	return 0;
}*/
