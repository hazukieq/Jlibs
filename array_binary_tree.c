#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include "JHeap.h"

static void jheap_set(JHeap* jheap,int index,void* val,int size){
	if(jheap==NULL) return;
	if(val==NULL) return;
	
	Any* any=anyOf(val,size);
	//anyLog(any,char*,s);
	
	if(any==NULL) return;
	jheap->arr[index]=any;
}

static Any* jheap_get(JHeap* jheap,int index){
	if(jheap==NULL||index<0||index>=jheap->size) return NULL;
	return jheap->arr[index];
}

static int _jheap_left(int i){
	return 2*i+1;
}

static int _jheap_right(int i){
	return 2*i+2;
}

static int _jheap_parent(int i){
	return (i-1)/2;
}




JHeap* jheap_init(int size){
	JHeap* jheap=malloc(sizeof(JHeap)+size*sizeof(Any*));
	if(jheap==NULL) return NULL;
	jheap->size=size;
	return jheap;
}

int jheap_size(JHeap* jheap){
	if(jheap==NULL) return 0;
	return jheap->size;
}

int jheap_isNone(JHeap *jh){
	if(jh==NULL||jh->size==0) return 0;
	return 1;
}

Any* jheap_pop(JHeap* jh){
	if(jh==NULL||jh->size==0) return NULL;
	return NULL;
}

void jheap_push(JHeap* jh,void* val,int size){
	if(jh==NULL||jh->size==0||val==NULL||size==0) return;

}

Any* jheap_peak(JHeap* jh){
	if(jh==NULL||jh->size==0) return NULL;
	return jh->arr[0];
}


/*
 * 记得手动释放内存
 */
Any** jheap_levelorder(JHeap* jheap){
	if(jheap==NULL) return NULL;

	Any** res=malloc(jheap->size*sizeof(Any*));
	if(res==NULL) return NULL;

	for(int i=0;i<jheap->size;i++){
		if(jheap->arr[i]!=NULL){
			res[i]=jheap->arr[i];
		}
	}
	return res;
}

#define PRE_ORDER (1<<0)
#define IN_ORDER (1<<1)
#define POST_ORDER (1<<2)
void jheap_dfs(JHeap* jheap,int i,char order,Any** res){
	if(jheap==NULL||res==NULL) return;
	if(jheap_get(jheap,i)==NULL) return;

	static int count=0;
	
	if(order==PRE_ORDER) 
		res[count++]=jheap_get(jheap,i);	
	jheap_dfs(jheap,_jheap_left(i),order,res);

	if(order==IN_ORDER) 
		res[count++]=jheap_get(jheap,i);
	jheap_dfs(jheap,_jheap_right(i),order,res);

	if(order==POST_ORDER)
		res[count++]=jheap_get(jheap,i);

	if(count==jheap->size) count=0;
}

/*
 * 记得手动释放内存
 */
Any** jheap_preorder(JHeap* jheap){
	Any** res=malloc(jheap->size*sizeof(Any*));
	if(res==NULL) return NULL;
	
	jheap_dfs(jheap,0,PRE_ORDER,res);
	return res;
}

/*
 * 记得手动释放内存
 */
Any** jheap_inorder(JHeap* jheap){
	Any** res=malloc(jheap->size*sizeof(Any*));
	if(res==NULL) return NULL;
	
	jheap_dfs(jheap,0,IN_ORDER,res);
	return res;
}

/*
 * 记得手动释放内存
 */
Any** jheap_postorder(JHeap* jheap){
	Any** res=malloc(jheap->size*sizeof(char*));
	if(res==NULL) return NULL;
	
	jheap_dfs(jheap,0,POST_ORDER,res);
	return res;
}

void jheap_release(JHeap* jheap){
	if(jheap==NULL) return;
	for(int i=0;i<jheap->size;i++){
		if(jheap->arr[i]) 
			anyFree(jheap->arr[i]);
	}
	free(jheap);
}


int main(void){
	JHeap* jheap=jheap_init(100);
	for(int i=0;i<100;i++){
		char s[9];
		sprintf(s,"%dhello",i);
		Any any=anyStr(s);
		printf("%s,%d\n",(char*)any.val,any.size);
		jheap_set(jheap,i,any.val,any.size);
	}

	Any** res=jheap_levelorder(jheap);
	//Any** res=jheap_preorder(jheap);
	//Any** res=jheap_inorder(jheap);
	//Any** res=jheap_postorder(jheap);
	for(int j=0;j<100;j++)
		anyLog((res[j]),char*,s)
	
	free(res);
	jheap_release(jheap);
	return 0;
}

