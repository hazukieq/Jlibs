#include "JList.h"
#include <malloc.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>


//循环链表方法实现
static void forEach(const JList* jlist,void (*executeFn)(void*)){
        if(jlist==NULL) return;
        if(jlist->len==0) return;
        Node* current=jlist->next;
        while(current){
                executeFn(current->obj);
                current=current->next;
        }
};


/**
 * size 对于数组而言，其size必须是类型大小*数组长度
 * depracted
 */
static void* applyMem(int size,char ltype){
        void* voidmem=NULL;
	switch(ltype){
		case 'i':
			voidmem=malloc(sizeof(int)*size);
			break;
		case 's':
			voidmem=malloc(sizeof(short)*size);
			break;
		case 'f':
			voidmem=malloc(sizeof(float)*size);
			break;
		case 'l':
			voidmem=malloc(sizeof(long)*size);
			break;
		case 'c':
			voidmem=malloc(sizeof(char)*(size+1));
			break;
		case 't':
		default:
			voidmem=malloc(size);
	}

        if(voidmem==NULL){
                printf("failed to malloc memory for void object.\n");
                return NULL;
        }
        return voidmem;
}



JList* jlist_init(char ltype){
        //创建头节点
        JList* jlist=(JList*) malloc(sizeof(JList));
        if(jlist==NULL){
                printf("out of memory wnhen malloc for init()\n");
                return NULL;
        }

        jlist->len=0;
        jlist->next=NULL;
	jlist->type=ltype;
        return jlist; 
}


static Node* createNode(void* obj,int obj_size,char obj_type){
        //创造一个新节点
        Node* new_node=malloc(sizeof(Node));
        //检查内存分配是否成功
        if(new_node==NULL){
                printf("failed to malloc memory for creating new node.\n");
                return NULL;
        }

        //节点初始化
        if(obj==NULL){
		goto break_c;
	}
        void* new_obj=applyMem(obj_size,obj_type);
	if(new_obj==NULL){
		goto break_c;
	}
	
	memcpy(new_obj,obj,obj_size);
	new_node->obj=new_obj;
	new_node->next=NULL;
        new_node->size=obj_size;
        return new_node;

break_c:
	free(new_node);
	return NULL;
}


void jlist_add(JList* jlist,void* obj,int obj_size){
        if(jlist==NULL||obj==NULL) return;
        
	//生成一个新节点
        Node* new_node=createNode(obj,obj_size,jlist->type);
        //检查新节点生成是否成功
        if(new_node==NULL) return;

        //头节点
        Node* current=jlist->next;


        //当头节点中的next指针为空时
        if(current==NULL){
                jlist->next=new_node;
                jlist->len++;
                return;
        }
        
        //采用后插法
        //需要找到倒数第二个元素(因为最后一个元素永远是NULL),并在其后面添加
        while(current->next!=NULL)
                current=current->next;
        
        //原来节点的next指针移交给新节点的next指针
        new_node->next= current->next;
        //原节点的next指针指向新节点
        current->next=new_node;
        
        //长度+1
        jlist->len++;
}


/**
 * 插入元素
 * 在某元素的位置前插入,同时将某元素后移一位,且长度+1
 */
void jlist_insert(JList* jlist,void* obj,int obj_size,int pos){
        if(jlist==NULL||obj==NULL) return;
        //检查待插入索引是否大于列表长度
        if(pos>jlist->len){
                printf("warning: insert_index(%d) which is unequal jlist length(%d),so adding opt is failed.\n",pos,jlist->len);
                return;
        }

        if(pos==jlist->len) {
                jlist_add(jlist,obj,obj_size);
                printf("append after last item...because index is out of JList bounds.\n");
                return;
        }

        //新节点
        Node* new_node=createNode(obj,obj_size,jlist->type);
        if(new_node==NULL) return;

        //当前节点
        Node* current=jlist->next;

        //说明列表未有初始化
        if(current==NULL&&jlist->len==0){
                jlist->next=new_node;
                jlist->len++;
                return;
        }



        //说明列表已经初始化
        //则需要将新节点置为第一个节点
        if(pos==0){
                new_node->next=jlist->next;
                jlist->next=new_node;
                jlist->len++;
                return;
        }

        //找到pos位置上的节点元素
        //因为current已经是第一个元素了
        //所以元素循环的起始点i=1
        for(int i=1;i<pos;i++){
                current=current->next;
        }

        new_node->next=current->next;
        current->next=new_node;

        jlist->len++;
}


void jlist_set(JList* jlist,void* obj,int obj_size,int pos){
        if(jlist==NULL||obj==NULL) return;

        if(jlist->next==NULL&&jlist->len==0){
                printf("error: jlist is uninitialize!\n");
                return;
        }

        if(pos>=jlist->len){
                printf("index is larger than jlist length.");
                return;
        }

        Node* current=jlist->next;
        if(pos==0){
                void* neo_obj=applyMem(obj_size,jlist->type);
                if(neo_obj==NULL){
                        printf("failed to malloc memory for object in jlist_set().\n");
                        return;
                }

                void* tmp=current->obj;
                free(tmp);
                tmp=NULL;

                memcpy(neo_obj,obj,obj_size);
                current->obj=neo_obj;
                return;
        }


        for(int i=1;i<pos;i++){
                current=current->next;
        }

        void* neo_obj=applyMem(obj_size,jlist->type);
        if(neo_obj==NULL){
                printf("failed to malloc memory for object in jlist_set().\n");
                return;

        }

        
        free(current->next->obj);

        memcpy(neo_obj,obj,obj_size);
        current->next->obj=neo_obj;
}


void* jlist_get(const JList* jlist,int pos){
        if(jlist==NULL) return NULL;

        if(jlist->next==NULL&&jlist->len==0){
                printf("error: JList is uninitialize!\n");
                return NULL;
        }

        pos=pos+1;
        if(pos>jlist->len){
                printf("index is larger than JList length.");
                return NULL;
        }
        
        if(pos<0)
                pos=jlist->len+pos;
        

        Node* current=jlist->next;
        
        if(pos==1) return current->obj;

        for(int i=1;i<pos;i++)
                current=current->next;

        return current->obj;
}

int jlist_getInt(void* obj){
	if(obj==NULL) return 0;
	return *(int*)obj;
}

float jlist_getFloat(void* obj){
	if(obj==NULL) return 0;
	return *(float*)obj;
}

short jlist_getShort(void* obj){
	if(obj==NULL) return 0;
	return *(short*)obj;
}

long jlist_getLong(void* obj){
	if(obj==NULL) return 0;
	return *(long*)obj;
}

char* jlist_getChars(void* obj){
	if(obj==NULL) return NULL;
	return (char*)obj;
}

void* jlist_getlast(const JList *jlist){
        if(jlist==NULL) return NULL;
        if(jlist->len==0) return  NULL;
        return jlist_get(jlist,jlist->len-1);
}


void jlist_rmObj(JList* jlist,void* obj){
        if(jlist==NULL||obj==NULL) return;

        Node* current=jlist->next;
        Node* nextz;

        if(current==NULL) return;
        
        //分为两种情况
        //1. 当 obj== 头节点.obj时, 直接将头节点的next指针指向next的next
        //2. 当 obj不等于头节点时, 则循环移除
        if(memcmp(current->obj,obj,current->size)==0){
                jlist->next=current->next;
                free(current->obj);
                free(current);
                current=jlist->next;
                jlist->len--;
        }


        //确保当前节点和下一个节点不为空
        while(current!=NULL&&current->next!=NULL){
            //下一个节点
            nextz=current->next;

            //当下一个节点和obj相等时
            //先将下个节点的next复制给当前节点的next
            //然后释放节点的占用内存,删除节点
            if(memcmp(nextz->obj,obj,nextz->size)==0){
                    //Node* tmp=current->next;
                    current->next=nextz->next;
                    free(nextz->obj);
                    free(nextz);
                    jlist->len--;
            }else{
            //继续循环:当前节点=下一个节点
            current=current->next;
            }
       }
 
}




void jlist_remove(JList* jlist,int pos){
        if(jlist==NULL) return;

        //检查索引是否大于列表长度
        if(pos>=jlist->len){
                printf("index has larger than jlist length(%d).\n",jlist->len);
                return;
        }


        //当长度为0且next指针为空时
        //说明列表未进行初始化
        if(jlist->next==NULL&&jlist->len==0) return;


        //当前节点
        Node* current=jlist->next;

        //当索引==0时
        if(pos==0){
                //头节点next指针指向下一个节点
                jlist->next=current->next;
                
                //释放第一个节点
                free(current->obj);
                free(current);
                
                current=jlist->next;
                jlist->len--;
                return;
        }


        //当索引>0时，先找到pos的上一个节点
        //找到pos位置上的节点元素
        //因为current已经是第一个元素了
        //所以元素循环的起始点i=1
        for(int i=1;i<pos;i++)
                current=current->next;
        

        //这个就是pos上的节点
        Node* tmp=current->next;

        if(tmp==NULL){
                printf("error: index is out of jlist bounds...\n");
                return;
        }
        
        //current指向pos的next节点
        current->next=current->next->next;
        
        //释放内存
        free(tmp->obj);
        free(tmp);
        
        //长度-1
        jlist->len--;
}


/*---释放内存方法---*/
void jlist_empty(JList* jlist){
        if(jlist==NULL) return;
        
        if(jlist->len==0 
           && jlist->next==NULL){
                printf("jlist_empty_info: jlist length is 0, so there is nothing to empty!\n");
                jlist->len=0;
                jlist->next=NULL;
                return;
        }

        Node* current=jlist->next;

        while(current!=NULL){
                Node* next=current->next;
                if(next==NULL) break;
                
                free(current->obj);
                free(current);
                current=next;
        }
	//防止最后一个节点没有释放内存
	if(current) {
		if(current->obj) free(current->obj);
		free(current);
	}
        
	jlist->len=0;
        jlist->next=NULL;
}

void jlist_release(JList* jlist){
    if(jlist==NULL) return;
    jlist_empty(jlist);
    free(jlist);
    jlist=NULL;
}


int jlist_include(const JList* jlist,void* obj,int obj_size){
        if(jlist==NULL||obj==NULL) return 0;

        if(jlist->next==NULL||jlist->len==0)
                return 0;

        int check=0;
        Node* current=jlist->next;
        while(current){
                if(memcmp(current->obj,obj,obj_size)==0){
                        check=1;
                        break;
                }
                current=current->next;
        }

        return check;
}

int jlist_contains(const JList* jlist,Node* node){
        if(jlist==NULL||node==NULL) return 0;

        if(jlist->next==NULL&&jlist->len==0)
                return 0;


        int check=0;
        Node* current=jlist->next;
        while(current){
                if(memcmp(current,node,sizeof(*current))==0){
                        check=1;
                        break;
                }
                current=current->next;
        }

        return check;
}


static int __jlist_node_equal(Node* v1,Node* v2){
        int fail=0;
        if(v1==NULL||v2==NULL) return fail;

        if(v1->next==v2->next
           &&(char*)v1->obj==(char*)v2->obj
           &&v1->size==v2->size) return 1;

        return fail;
}
int jlist_indexof(const JList *jlist, Node *node){
        if(jlist==NULL||node==NULL) return -1;
        int index=0;
        jlist_for(jlist, obj){
                if(__jlist_node_equal(node,obj)==1) break;
                index++;
        }

        return index;
}


void jlist_reverse(JList* jlist){
        if(jlist==NULL||jlist->len==0) return;

        Node* nhead=NULL;
        Node* ncur=NULL;

        Node* next=NULL;
        Node* cur=jlist->next;
        while(cur){
                next=cur->next;

                //假设nhead是头节点,且不为NULL
                //ncur是新添节点
                ncur=cur;
                ncur->next=nhead;
                nhead=ncur;

                cur=next;
        }

        //将原来的头节点替换掉
        jlist->next=nhead;
}

//只是将列表的最后一个指向了新列表的头节点
//并不是真正的复制合并
//但两个列表内存都应该释放，请用 if(...) free(...)
//jlist1合并给jlist
void jlist_merge(JList* jlist1,JList* jlist){
        if(jlist1==NULL||jlist==NULL||jlist1->type!=jlist->type) return; 
        //找到jlist表最后一个节点
	Node* last=jlist->next;
        while(last->next)
                last=last->next;
        if(last){
		printf("merge jlist successfully...\n");
		last=jlist1->next;
		jlist->len+=jlist1->len;
	}
}

void jlist_test(){
	//PASS
	JList* jlist=jlist_init(LCHAR);
	jlist_add(jlist,"hello",5);
	jlist_set(jlist,"hello,world",12,0);
	printf("lastItem:%s\n",(char*)jlist_getlast(jlist));
	
	jlist_rmObj(jlist,"hello");
	jlist_for(jlist,node){
		char* val=jlist_getChars(node->obj);
		printf("%s\n",jlist_getChars(node->obj));
	}
	jlist_release(jlist);
}


/*int main(void){
	jlist_test();
	return 0;
}*/
