#include "JStack.h"

static JNode* __jst_create_snode(void* data,int size){
        if(data==NULL||size==0) return NULL;

	JNode* jnode=malloc(sizeof(JNode));
        if(jnode==NULL) return NULL;

        void* jdata=malloc(size);
        if(jdata==NULL){
		if(jnode) free(jnode);
		return NULL;
	}
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


void jstack_empty(JStack* jst){
        if(jst==NULL) return;
        
        JNode* current=jst->top;
        while(current){
                JNode* next=current->next;
                free(current->data);
                free(current);
                current=next;
        }

        jst->len=0;
        jst->top=NULL;
}

void jstack_release(JStack* jst){
        if(jst==NULL) return;
        
        jstack_empty(jst);
        free(jst);
        jst=NULL;
}


int jstack_len(JStack* jst){
        if(jst==NULL) return -1;
        return jst->len;
}

int jstack_isnone(JStack* jst){
        if(jst==NULL||jst->len==0) return 1;
        else return 1;
}

void* jstack_top(JStack* jst){
        if(jst==NULL||jst->len==0) return NULL;
        return jst->top->data;
}

void jstack_pop(JStack* jst){
        if(jst==NULL||jst->len==0) return;

        JNode* current=jst->top;
        if(current==NULL) return;

        JNode* next=current->next;
	
	free(current->data);
        free(current);
        
	jst->top=next;
        jst->len--;
}

void* jstack_popobj(JStack* jst){
        if(jst==NULL||jst->len==0) return NULL;

        JNode* current=jst->top;
        if(current==NULL) return NULL;

        JNode* next=current->next;
        void* data=malloc(current->size);
	if(data==NULL) return NULL;
	memcpy(data,current->data,current->size);
	
	free(current->data);
        free(current);
        
	jst->top=next;
        jst->len--;

        return data;
}
void jstack_popobj_free(void *void_obj){
	if(void_obj) free(void_obj);
}
void jstack_pop_freeAll(void** alls,int len){
	if(len==0) return;
	for(int i=0;i<len;i++){
		if(alls[i]) free(alls[i]);
	}
}

//size包含字符结束标志\0,若使用strlen则请+1
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
        if(jst==NULL||jst->len==0) return;

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
        //--PASS jstack_init--/
	//--PASS jstack_push--/
	//--PASS jstack_top--/
	//--PASS jstack_pop--/
	//--PASS jstack_popobj&jstack_popobj_free--/
	//--PASS jstack_reverse--/
	JStack* jst=jstack_init();

        jstack_push(jst,"hello",6);
        int i=0;
        while(++i<101){
                jstack_push(jst,&i,sizeof(int));
        }

        jstack_reverse(jst);
        printf("jstack_top->%d\n",*((int*)jstack_top(jst)));

	jstack_pop(jst);
	void* s=jstack_popobj(jst);
        printf("_rever=> %s\n",(char*)s);
        jstack_pop_auto(s);
	jstack_release(jst);
}

/*int main(){
	jstack_test();
	return 0;
}*/
