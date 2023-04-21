#include "JStack.h"

static JNode* __jst_create_snode(void* data,int size){
        JNode* jnode=malloc(sizeof(JNode));
        if(jnode==NULL) return NULL;

        void* jdata=malloc(size);
        void* check=memcpy(jdata,data,size);
        if(check==NULL) return NULL;
        jnode->data=jdata;
        
        jnode->size=size;
        jnode->next=NULL;
        return jnode;
}


JStack* jstack_init(){
        JStack* init=malloc(sizeof(JStack));
        if(init==NULL) return NULL;
        
        init->top=NULL;
        init->len=0;
        return init;
}


void jstack_release(JStack* jst){
        if(jst==NULL) return;
        
        jstack_empty(jst);
        free(jst);
        jst=NULL;
}

void jstack_empty(JStack* jst){
        if(jst==NULL) return;
        
        JNode* current=jst->top;
        JNode* next=NULL;

        while(--jst->len>0){
                next=current->next;
                free(current->data);
                free(current);
                current=next;
        }

        jst->len=0;
        jst->top=NULL;
}


int jstack_len(JStack* jst){
        if(jst==NULL) return -1;
        return jst->len;
}


void* jstack_pop(JStack* jst){
        if(jst==NULL) return NULL;

        JNode* current=jst->top;
        JNode* next=current->next;

        void* data=current->data;
        free(current);
        jst->top=next;
        jst->len--;

        return data;
}

void jstack_push(JStack* jst,void* data,int size){ 
        if(jst==NULL||data==NULL) return;

        JNode* new_node=__jst_create_snode(data, size);
        if(new_node==NULL) return;

        JNode* current=jst->top;
        new_node->next=current;
        jst->top=new_node;
        jst->len++;
}

void jstack_putnode(JStack* jst,JNode* node){
        if(jst==NULL||node==NULL) return;

        JNode* current=jst->top; 
        node->next=current;
        jst->top=node;
        jst->len++;
}

void jstack_reverse(JStack* jst){
        if(jst==NULL) return;

        JNode* current=jst->top;
        JNode* next=NULL;
       
        //新的头节点、新节点
        //将原来的链表倒序排列
        JNode* nhead=NULL;
        JNode* nnew=NULL;
        while(current){
                next=current->next;
                
                nnew=current;
                nnew->next=nhead;
                nhead=nnew;

                current=next;
        }

        //将倒序排列好的链表复制给原链表栈头节点
        jst->top=nhead;
}

void jstack_test(){
        JStack* jst=jstack_init();

        int i=0;
        while(++i<101){
                jstack_push(jst,&i,sizeof(int));
        }

        jstack_reverse(jst);
        while(jst->len>0)
                printf("_rever=> %d\n",*((int*)jstack_pop(jst)));
        
        jstack_release(jst);
}

