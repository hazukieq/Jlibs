#include "JList.h"

//循环链表方法实现
static void forEach(const List* list,void (*executeFn)(void*)){
        if(list==NULL) return;
        if(list->len==0) return;
        Node* current=list->next;
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



List* list_init(char ltype){
        //创建头节点
        List* list=(List*) malloc(sizeof(List));
        if(list==NULL){
                printf("out of memory wnhen malloc for init()\n");
                return NULL;
        }

        list->len=0;
        list->next=NULL;
	list->type=ltype;
        return list; 
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


void list_add(List* list,void* obj,int obj_size){
        if(list==NULL||obj==NULL) return;
        
	//生成一个新节点
        Node* new_node=createNode(obj,obj_size,list->type);
        //检查新节点生成是否成功
        if(new_node==NULL) return;

        //头节点
        Node* current=list->next;


        //当头节点中的next指针为空时
        if(current==NULL){
                list->next=new_node;
                list->len++;
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
        list->len++;
}


/**
 * 插入元素
 * 在某元素的位置前插入,同时将某元素后移一位,且长度+1
 */
void list_insert(List* list,void* obj,int obj_size,int pos){
        if(list==NULL||obj==NULL) return;
        //检查待插入索引是否大于列表长度
        if(pos>list->len){
                printf("warning: insert_index(%d) which is unequal list length(%d),so adding opt is failed.\n",pos,list->len);
                return;
        }

        if(pos==list->len) {
                list_add(list,obj,obj_size);
                printf("append after last item...because index is out of List bounds.\n");
                return;
        }

        //新节点
        Node* new_node=createNode(obj,obj_size,list->type);
        if(new_node==NULL) return;

        //当前节点
        Node* current=list->next;

        //说明列表未有初始化
        if(current==NULL&&list->len==0){
                list->next=new_node;
                list->len++;
                return;
        }



        //说明列表已经初始化
        //则需要将新节点置为第一个节点
        if(pos==0){
                new_node->next=list->next;
                list->next=new_node;
                list->len++;
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

        list->len++;
}


void list_set(List* list,void* obj,int obj_size,int pos){
        if(list==NULL||obj==NULL) return;

        if(list->next==NULL&&list->len==0){
                printf("error: List is uninitialize!\n");
                return;
        }

        if(pos>=list->len){
                printf("index is larger than List length.");
                return;
        }

        Node* current=list->next;
        if(pos==0){
                void* neo_obj=applyMem(obj_size,list->type);
                if(neo_obj==NULL){
                        printf("failed to malloc memory for object in list_set().\n");
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

        void* neo_obj=applyMem(obj_size,list->type);
        if(neo_obj==NULL){
                printf("failed to malloc memory for object in list_set().\n");
                return;

        }

        
        free(current->next->obj);

        memcpy(neo_obj,obj,obj_size);
        current->next->obj=neo_obj;
}


void* list_get(const List* list,int pos){
        if(list==NULL) return NULL;

        if(list->next==NULL&&list->len==0){
                printf("error: List is uninitialize!\n");
                return NULL;
        }

        pos=pos+1;
        if(pos>list->len){
                printf("index is larger than List length.");
                return NULL;
        }
        
        if(pos<0)
                pos=list->len+pos;
        

        Node* current=list->next;
        
        if(pos==1) return current->obj;

        for(int i=1;i<pos;i++)
                current=current->next;

        return current->obj;
}

int list_getInt(void* obj){
	if(obj==NULL) return 0;
	return *(int*)obj;
}

float list_getFloat(void* obj){
	if(obj==NULL) return 0;
	return *(float*)obj;
}

short list_getShort(void* obj){
	if(obj==NULL) return 0;
	return *(short*)obj;
}

long list_getLong(void* obj){
	if(obj==NULL) return 0;
	return *(long*)obj;
}

char* list_getChars(void* obj){
	if(obj==NULL) return NULL;
	return (char*)obj;
}

void* list_getlast(const List *list){
        if(list==NULL) return NULL;
        if(list->len==0) return  NULL;
        return list_get(list,list->len-1);
}


void list_rmObj(List* list,void* obj){
        if(list==NULL||obj==NULL) return;

        Node* current=list->next;
        Node* nextz;

        if(current==NULL) return;
        
        //分为两种情况
        //1. 当 obj== 头节点.obj时, 直接将头节点的next指针指向next的next
        //2. 当 obj不等于头节点时, 则循环移除
        if(memcmp(current->obj,obj,current->size)==0){
                list->next=current->next;
                free(current->obj);
                free(current);
                current=list->next;
                list->len--;
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
                    list->len--;
            }else{
            //继续循环:当前节点=下一个节点
            current=current->next;
            }
       }
 
}




void list_remove(List* list,int pos){
        if(list==NULL) return;

        //检查索引是否大于列表长度
        if(pos>=list->len){
                printf("index has larger than List length(%d).\n",list->len);
                return;
        }


        //当长度为0且next指针为空时
        //说明列表未进行初始化
        if(list->next==NULL&&list->len==0) return;


        //当前节点
        Node* current=list->next;

        //当索引==0时
        if(pos==0){
                //头节点next指针指向下一个节点
                list->next=current->next;
                
                //释放第一个节点
                free(current->obj);
                free(current);
                
                current=list->next;
                list->len--;
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
                printf("error: index is out of List bounds...\n");
                return;
        }
        
        //current指向pos的next节点
        current->next=current->next->next;
        
        //释放内存
        free(tmp->obj);
        free(tmp);
        
        //长度-1
        list->len--;
}


/*---释放内存方法---*/
void list_empty(List* list){
        if(list==NULL) return;
        
        if(list->len==0 
           && list->next==NULL){
                printf("list_empty_info: list length is 0, so there is nothing to empty!\n");
                list->len=0;
                list->next=NULL;
                return;
        }

        Node* current=list->next;

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
        
	list->len=0;
        list->next=NULL;
}

void list_release(List* list){
    if(list==NULL) return;
    list_empty(list);
    free(list);
    list=NULL;
}


int list_include(const List* list,void* obj,int obj_size){
        if(list==NULL||obj==NULL) return 0;

        if(list->next==NULL||list->len==0)
                return 0;

        int check=0;
        Node* current=list->next;
        while(current){
                if(memcmp(current->obj,obj,obj_size)==0){
                        check=1;
                        break;
                }
                current=current->next;
        }

        return check;
}

int list_contains(const List* list,Node* node){
        if(list==NULL||node==NULL) return 0;

        if(list->next==NULL&&list->len==0)
                return 0;


        int check=0;
        Node* current=list->next;
        while(current){
                if(memcmp(current,node,sizeof(*current))==0){
                        check=1;
                        break;
                }
                current=current->next;
        }

        return check;
}


static int __list_node_equal(Node* v1,Node* v2){
        int fail=0;
        if(v1==NULL||v2==NULL) return fail;

        if(v1->next==v2->next
           &&(char*)v1->obj==(char*)v2->obj
           &&v1->size==v2->size) return 1;

        return fail;
}
int list_indexof(const List *list, Node *node){
        if(list==NULL||node==NULL) return -1;
        int index=0;
        list_for(list, obj){
                if(__list_node_equal(node,obj)==1) break;
                index++;
        }

        return index;
}


void list_reverse(List* list){
        if(list==NULL||list->len==0) return;

        Node* nhead=NULL;
        Node* ncur=NULL;

        Node* next=NULL;
        Node* cur=list->next;
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
        list->next=nhead;
}

//只是将列表的最后一个指向了新列表的头节点
//并不是真正的复制合并
//但两个列表内存都应该释放，请用 if(...) free(...)
//list1合并给list
void list_merge(List* list1,List* list){
        if(list1==NULL||list==NULL||list1->type!=list->type) return; 
        //找到list表最后一个节点
	Node* last=list->next;
        while(last->next)
                last=last->next;
        if(last){
		printf("merge list successfully...\n");
		last=list1->next;
		list->len+=list1->len;
	}
}

void list_test(){
	//PASS
	List* list=list_init(LCHAR);
	list_add(list,"hello",5);
	list_set(list,"hello,world",12,0);
	printf("lastItem:%s\n",(char*)list_getlast(list));
	
	list_rmObj(list,"hello");
	list_for(list,node){
		char* val=list_getChars(node->obj);
		printf("%s\n",list_getChars(node->obj));
	}
	list_release(list);
}


/*int main(void){
	list_test();
	return 0;
}*/
