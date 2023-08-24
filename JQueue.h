#ifndef __JQUEUE_H__
#define __JQUEUE_H__

#define jque_for(jq,obj) \
        Jquenod* obj=NULL;\
        for(obj=jq->front;obj;obj=obj->next)

struct _queue_node{
        struct _queue_node *next;
        void* data;
        int size;
};
typedef struct _queue_node Jquenod;

struct _queue{
        Jquenod* rear;
        Jquenod* front;
        int len;
};
typedef struct _queue Jque;

static Jquenod* __jq_create_node(void* data,int size);

Jque* jque_init();

int jque_isnone(Jque* jq);

void* jque_top(Jque* jq);
void* jque_end(Jque* jq);

void jque_empty(Jque* jq);
void jque_release(Jque* jq);

void jque_push(Jque* jq,void* data,int size);

//记得调用jque_pop_free释放内存!!!
void* jque_pop(Jque* jq);
void jque_pop_free(void* pop_obj);

void jque_test();
#endif
