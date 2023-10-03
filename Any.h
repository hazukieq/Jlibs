#ifndef __ANY_H__
#define __ANY_H__
#include <string.h>

struct _Any{
	int capacity; //申请内存容量
	int size;//当前储存长度
	char check;//是否是Any结构体判断标志
	void* val;//储存数据指针
};

typedef struct _Any Any;

static Any* _new(void* val,int size);

//初始化
Any* anyOf(void* val,int size);
//字符化->any
#define anyStr(val) (Any){strlen((char*)(val))+24,strlen((char*)(val))+1,0x01,(void*)(val)}
//字符化->any*
#define anyTr(val) anyOf(val,(int)(strlen((char*)(val))+1))
//任意化->any
#define anyI(val,size) (Any){size+23,size,0x01,(void*)val}

//更新Any值
void anySet(Any* any,void* val,int size);

Any* anyCopy(Any* any);
//清空
void anyClear(Any* any);

//释放内存
void anyFree(Any* any);

void* anyVal(Any* any);
int anySize(Any* any);


//结构体打印
#define anyLog(any,val_type,val_fmt) \
	if(any!=NULL||any->check==0x01){\
		printf("anyLog:\033[0;32m val<%" #val_fmt ">,size<%d>\033[0m\n",(val_type)any->val,any->size);\
	}

//得到结构体指针
#define anyPtr(any_val) ptr(any_val,Any,val)

#define anyAuto(...) anyFrees((Any*[]){__VA_ARGS__},sizeof((Any*[]){__VA_ARGS__})/sizeof(Any*))
void anyFrees(Any* anys[],int len);

void any_test();
#endif
