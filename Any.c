#include "Any.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "common.h"

#define INITED 0x01
#define INIT 0x00

static Any* _new(void* val,int size){
	Any* any=malloc(sizeof(Any));
	
	any->capacity=23;
	any->check=INITED;
	any->size=size;
	any->val=NULL;

	if(size>(any->capacity)) any->capacity=size+23;
	any->val=malloc(any->capacity);
	if(any->val)
		memcpy(any->val,val,size);	
	return any;
}

/*Any anyBy(void* val,int size){
	if(val==NULL||size==0){
		Any* any=malloc(sizeof(Any));
		any->size=0;
		any->capacity=23;
		any->val=NULL;
		return *any;
	}
	return *_new(val,size);
}*/

//初始化
Any* anyOf(void* val,int size){
	if(val==NULL||size==0) return NULL;
	return _new(val,size);
}

void anySet(Any* any,void* val,int size){
	if(any==NULL||any->check!=INITED) return;

	int new_len=any->capacity<size?size+23:any->capacity;
	
	Any* check=realloc(any->val,new_len);
	if(check==NULL) return;
	
	any->val=check;
	any->capacity=new_len;

	memcpy(any->val,val,size);
	any->size=size;
}

Any* anyCopy(Any* any){
	if(any==NULL||any->check!=INITED) return NULL;
	Any* new_any=_new(any->val,any->size);
	return new_any;
}

//清空
void anyClear(Any* any){
	if(any==NULL||any->check!=INITED) return;
	if(any->val) free(any->val);
	any->val=NULL;
	any->size=0;
	any->capacity=23;
}

//释放内存
void anyFree(Any* any){
	if(any==NULL||any->check!=INITED) return;
	if(any->val) free(any->val);
	free(any);
}

void anyFrees(Any* anys[],int len){
	if(anys==NULL) return;
	for(int i=0;i<len;i++)
		anyFree(anys[i]);
}

//得到值
void* anyVal(Any* any){
	if(any==NULL||any->check!=INITED||any->val==NULL) return "null";
	return any->val;
}
//得到值储存长度
int anySize(Any* any){
	if(any==NULL||any->check!=INITED) return 0;
	return any->size;
}



void any_test(){
	Any* any=anyOf("hello,world!",13);
	
	Any* any_=anyCopy(any);
	printf("any_: %s,%d,%d\n",(char*)any_->val,any_->size,any_->capacity);
	
	anySet(any,"hello",6);
	anyClear(any);
	printf("%s\n",(char*)anyVal(any));
	//anyFree(any);
	anyAuto(any,any_);
}

/*int main(void){
	any_test();
	return 0;
}*/

