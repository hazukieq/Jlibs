#include "JQueue.h"

static Jquenod* __jq_create_node(void* data,int size){
        if(data==NULL||size==0) return NULL;

        void* ndata=malloc(size);
        if(ndata==NULL) return NULL;
        void* check=memcpy(ndata,data,size);
        if(check==NULL) return NULL;
        
        Jquenod* jqnod=malloc(sizeof(Jquenod));
        if(jqnod==NULL) return NULL;
        jqnod->data=ndata;
        jqnod->next=NULL;

        return jqnod;
}


Jque* jque_init(){
        Jque* jq=malloc(sizeof(Jque));
        if(jq==NULL) return NULL;

        jq->front=jq->rear=NULL;
        jq->len=0;

        return jq;
}

void* jque_top(Jque* jq){
        if(jq==NULL||jq->len==0) return NULL;
        void* data=jq->front->data;
        return data;
}

void* jque_end(Jque* jq){
        if(jq==NULL||jq->len==0) return NULL;
        void* data=jq->rear->data;
        return data;
}

int jque_isnone(Jque* jq){
        if(jq==NULL||jq->len==0) return 1;
        return 0;
}

void jque_empty(Jque* jq){
        if(jq==NULL||jq->len==0) return;
        
        Jquenod* current=jq->front;
        while(current){
                Jquenod* next=current->next;
                free(current->data);
                free(current);
                current=next;
        }
        jq->len=0;
}


void jque_release(Jque* jq){
        if(jq==NULL||jq->len==0) return;
        jque_empty(jq);
        free(jq);
        jq=NULL;
}


void jque_push(Jque* jq,void* data,int size){
        if(jq==NULL||data==NULL) return;
        
        Jquenod* jqnod=__jq_create_node(data,size);
        if(jqnod==NULL) return;
       
        //如果头尾元素为空的话
        //则需要将新节点赋值给头尾元素
        if(jq->rear==NULL&&jq->front==NULL) {
                jq->front=jq->rear=jqnod;
                jq->len++;
                return;
        }
        
        Jquenod* cur_rear=jq->rear;
        cur_rear->next=jqnod;
        jq->rear=jqnod;
        jq->len++;
}

void* jque_pop(Jque* jq){
        if(jq==NULL||jq->len==0) return NULL;
        
        Jquenod* cur_front=jq->front;
        Jquenod* next=cur_front->next;
        void* data=cur_front->data;
        
        //弹出首个元素
        //把第二个元素置为首个元素
        //更新头节点
        jq->front=next;
        //当队列只有一个元素时
        if(jq->len==1) jq->front=jq->rear;
        free(cur_front);
        jq->len--;
        return data;
}

void jque_test(){
        Jque* jq=jque_init();

        for(int i=0;i<50;i++)
                jque_push(jq,"hello,world",12);
        
        int l=0;
        jque_for(jq,cur){
                if(cur!=NULL){
                        printf("%d_item is <%s>\n",l,(char*)cur->data);
                        l++;
                }
        }
        
        jque_push(jq,"h",2);
        printf("jq_front is %s,rear is %s\n",(char*)(jq->front->data),(char*)(jq->rear->data));
        
        void* d=jque_pop(jq);
        printf("item is %s,jq_len_isnone is %d\n",(char*)d,jque_isnone(jq));
        printf("top<%s>,end<%s>\n",(char*)jque_top(jq),(char*)jque_end(jq));
        jque_empty(jq);
        jque_release(jq);
}

int main(void){
        jque_test();
        return 0;
}

