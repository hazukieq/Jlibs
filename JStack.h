#ifndef __JSTACK_H_
#define __JSTACK_H_

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

//请注意 obj 是一个节点，不是实际数据！
#define jstack_for(jstack,obj) \
        JNode* obj=NULL;\
        for(obj=jstack->top;obj!=NULL;obj=obj->next)
#define EXPAND(...)  (void*[]){__VA_ARGS__},sizeof((void*[]){__VA_ARGS__})/sizeof(void*)
#define jstack_pop_auto(...) jstack_pop_freeAll(EXPAND(__VA_ARGS__))

struct _snode{
        void* data;
        int size;
        struct _snode* next;
};

typedef struct _snode JNode;

struct _stack{
        int len;
        JNode* top;
};

typedef struct _stack JStack;


static JNode* __jst_create_snode(void* data,int size);


JStack* jstack_init();
void jstack_release(JStack* jst);
void jstack_empty(JStack* jst);
int jstack_len(JStack* jst);
int jstack_isnone(JStack* jst);

void* jstack_top(JStack* jst);
void jstack_pop(JStack* jst);
//记得手动释放内存!!
void* jstack_popobj(JStack* jst);
void jstack_popobj_free(void* void_obj);
void jstack_pop_freeAll(void** alls,int len);
void jstack_push(JStack* jst,void* data,int size);

void jstack_reverse(JStack* jst);

void jstack_test();
#endif
