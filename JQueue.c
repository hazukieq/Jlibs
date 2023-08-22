#include "JQueue.h"

static Jquenod* __jq_create_node(void* data,int size){
	if(data==NULL||size==0) return NULL;

	Jquenod* jqnod=malloc(sizeof(Jquenod));
        if(jqnod==NULL) return NULL;

        void* ndata=malloc(size);
	if(ndata==NULL) {
		free(jqnod);
		return NULL;
	}
	void* check=memcpy(ndata,data,size);
        if(check==NULL){
		free(ndata);
		free(jqnod);
		return NULL;
	}
	
        jqnod->data=ndata;
	jqnod->size=size;
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

	jq->rear=jq->front=NULL;
	jq->len=0;
}


void jque_release(Jque* jq){
        if(jq==NULL) return;
        jque_empty(jq);
        if(jq) free(jq);
}


//size包含字符结束标志\0,若使用strlen则请+1
void jque_push(Jque* jq,void* data,int size){
        if(jq==NULL||data==NULL) return;
        
        Jquenod* jqnod=__jq_create_node(data,size);
	if(jqnod==NULL) return;

	if(jq->front==NULL&&jq->rear==NULL){
		jq->front=jq->rear=jqnod;
		jq->len++;
		return;
	}

	if(jq->front==NULL||jq->rear==NULL){
		printf("front or rear is null.\n");
		return;
	}
	jq->rear->next=jqnod;
	jq->rear=jq->rear->next;
	jq->len++;
}

void* jque_pop(Jque* jq){
        if(jq==NULL||jq->len==0) return NULL;
        
        Jquenod* cur=jq->front;
	if(cur==NULL) return NULL;
	
	//弹出首个元素
        //把第二个元素置为首个元素
        //更新头节点
        //当队列只有一个元素时
        jq->front=cur->next;
	if(jq->rear==cur) jq->rear=jq->front;
        
	//释放内存
	void* data=cur->data;
	free(cur);
        jq->len--;
        return data;
}
void jque_pop_free(void* pop_obj){
	if(pop_obj) free(pop_obj);
}

void jque_test(){
        //--PASS jque_init--/
	//--PASS jque_release--/
	//--PASS jque_push--/
	//--PASS jque_for--/
	//--PASS jque_top--/
	//--PASS jque_end--/
	//--PASS jque_empty--/
	//--PASS jque_pop--/
	//--PASS jque_none--/
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
        
        jque_push(jq,"h",strlen("h")+1);
        printf("jq_front is %s,rear is %s\n",(char*)(jq->front->data),(char*)(jq->rear->data));
        
        void* d=jque_pop(jq);
	printf("item is %s,jq_len_isnone is %d\n",(char*)d,jque_isnone(jq));
        printf("top<%s>,end<%s>\n",(char*)jque_top(jq),(char*)jque_end(jq));
        jque_empty(jq); 
	jque_pop_free(d);
	jque_release(jq);
}

/*int main(void){
        jque_test();
        return 0;
}*/

