#include <stdio.h>
#include <assert.h>
#include "Any.h"
#include "JHeap.h"
#include "common.h"
#include <malloc.h>
#include "JSha256.h"
#define _hash(node) _hash_fn(anyVal(node),anySize(node))

static int _hash_fn(void* val,int size){
	unsigned char* hash;
	jsha_hash(val,size,&hash);
	
	unsigned int hash_int=*((unsigned int*)hash);
	if(hash) free(hash);
	
	return hash_int;
}

static Any* _get(JHeap* jh,int i){
	if(i>jh->capacity){
		loge("Warning: index<%d> is greater than jh'size<%d>!\nYou must take care of this!",i,jh->size);
		acheck(i<jh->capacity,0);
	}
	return jh->arr[i];
}
static int _left(JHeap* jh,int i){
	acheck(i<jh->capacity,0);
	return 2*i+1;
}

static int _right(JHeap* jh,int i){
	acheck(i<jh->capacity,0);
	return 2*i+2;
}

static int _parent(JHeap* jh,int i){
	acheck(i<jh->capacity,0);
	return (i-1)/2;
}

static void _shiftUp(JHeap* jh,int i){
	while(1){
		int p=_parent(jh,i);
		if(p<0||jh->arr[i]<=jh->arr[p]) break;
		
		Any* tmp=jh->arr[i];
		jh->arr[i]=jh->arr[p];
		jh->arr[p]=tmp;
		i=p;
	}
}

static int _findMax(JHeap* jh,int l_index,int p_index,int r_index){
	int max=p_index;
	Any* l_node=_get(jh,l_index);
	Any* r_node=_get(jh,r_index);
	Any* p_node=_get(jh,p_index);

	if(l_index<jh->size&&_hash(l_node)>_hash(p_node)){
		max=l_index;
	}

	if(r_index<jh->size&&_hash(r_node)>_hash(p_node)){
		max=r_index;
	}

	return max;
}

static void _shiftDown(JHeap* jh,int p_index){
	while(1){
		int l_index=_left(jh,p_index);
		int r_index=_right(jh,p_index);
		int max=p_index;
		_findMax(jh,l_index,p_index,r_index);
		if(max==p_index) break;

		Any* tmp=jh->arr[p_index];
		jh->arr[p_index]=jh->arr[max];
		jh->arr[max]=tmp;
	}
}

JHeap* jheap_init(int capacity){
	if(capacity<=30) capacity=30;
	JHeap* jheap=malloc(_S(JHeap)+_S(Any*)*capacity);
	acheck(jheap,0);

	jheap->capacity=capacity;
	jheap->size=0;
	
	return jheap;
}

Any* jheap_pop(JHeap* jh){
	ifcheck(jh->size==0,NULL);
	Any* tmp=jh->arr[0];
	jh->arr[0]=jh->arr[jh->size-1];
	jh->arr[jh->size-1]=tmp;

	Any* val=jh->arr[jh->size-1];
	jh->size--;

	_shiftDown(jh,0);
	return val;
}

//jheap_push_any
void jheap_push_any(JHeap* jh,Any* any){
	acheck(jh,);
	if(jh->size==jh->capacity){
		loge("heap is full,size<%d> must be less than max_capacity<%d>!",jh->size,jh->capacity);
		return;
	}
	jh->arr[jh->size]=any;
	jh->size++;

	_shiftUp(jh,jh->size-1);
}

void jheap_push(JHeap* jh,void* val,int size){
	jheap_push_any(jh,anyOf(val,size));
}

Any* jheap_peak(JHeap* jh){
	acheck(jh,NULL);
	return anyCopy(jh->arr[0]);
}

int jheap_size(JHeap* jh){
	acheck(jh,0);
	return jh->size;
}

int jheap_isNone(JHeap* jh){
	acheck(jh->capacity>0&&jh->size>0,FALSE);
	return TRUE;
}

void jheap_empty(JHeap* jh){
	for(int i=0;i<jh->size;i++)
		anyFree(jh->arr[i]);
	jh->size=0;
}

void jheap_release(JHeap* jh){
	acheck(jh,);
	jheap_empty(jh);
	free(jh);
}


int main(void){
	JHeap* jh=jheap_init(10);	
	jheap_push_any(jh,anyTr("hello"));
	jheap_push_any(jh,anyTr("world"));
	jheap_push_any(jh,anyTr("hello,world"));
	
	logi((jh->size),d,"");
	
	Any* any=jheap_peak(jh);
	anyLog(any,char*,s);
	logc("jh_pop: %s",(char*)anyVal(any));
	anyFree(any);
	jheap_release(jh);
	return 0;
}

