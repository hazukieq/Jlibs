#ifndef __JSTACK_H_
#define __JSTACK_H_

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

//请注意 obj 是一个节点，不是实际数据！
#define jstack_for(jstack,obj) \
        JNode* obj=NULL;\
        for(obj=jstack->top;obj!=NULL;obj=obj->next)

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
void* jstack_pop(JStack* jst);
void jstack_push(JStack* jst,void* data,int size);

void jstack_reverse(JStack* jst);

void jstack_test();
#endif
