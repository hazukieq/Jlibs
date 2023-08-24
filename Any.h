#ifndef __ANY_H__
#define __ANY_H__
struct _Any{
	int capacity; //申请内存容量
	int size;//当前储存长度
	char check;//是否是Any结构体判断标志
	void* val;//储存数据指针
};

typedef struct _Any Any;

static Any* _new();
static void _del(Any* any);
static void _empty(Any* any);

//初始化
void* anyOf(void* val,int size);

//清空
void anyClear(Any* any);

//释放内存
void anyFree(Any* any);

//得到值
void* any(Any* any);

//得到结构体指针
#define anyPtr(any_val) ptr(any_val,Any,val)

#define assertAny(any_val,ret) \
if(anyPtr(any_val)->check!=0x01){\
	loge("this object is not initialized properly...")\
	return ret;\
}

#define anyAuto(...) anyFrees((Any*[]){__VA_ARGS__},sizeof((Any*[]){__VA_ARGS__})/sizeof(Any))
void anyFrees(Any* anys[],int len);
#endif
