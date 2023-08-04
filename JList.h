#ifndef __JLIST_H_
#define __JLIST_H_

#include <stdio.h>
#include <malloc.h>


#define list_foreach(list,execueteFn) \
       for(Node* item=list->next;item;item=item->next)\
            if(item->obj!=NULL) executeFn(item->obj);

//请注意 node 是一个节点，不是实际数据！
#define list_for(list,node) \
        for(Node* node=list->next;node;node=node->next)

#define List2(list) \
        for(Node* it=list->next;it;it=it->next)

#define LINT 'i'
#define LFLOAT 'f'
#define LSHORT 's'
#define LLONG 'l'
#define LCHAR 'c'
#define LSTRUCT 't'

/*union T_VALS{
	int i;
	float f;
	short e;
	long l;
	char* c;
	void* t;
};*/


//链表普通节点
typedef struct ListNode{
        //每个元素占用的空间大小不一
        int size;
        struct ListNode* next;
	void* obj;
} Node;



//链表头节点
//存储链表关键信息
typedef struct ListHead{
        char type;//请使用LType中任意值
        int len;
        Node* next;
} List;


/**
 * 列表初始化
 * ListType 枚举
 * size 数据类型大小
 * return 返回一个结构体
 */
List* list_init(char ltype);

//根据索引插入元素
void list_insert(List* list,void* obj,int obj_size,int pos);

//在尾部添加元素
void list_add(List* list,void* obj,int obj_size);

//修改元素值
//void list_setObj(List* list,void* obj,int obj_size);
void list_set(List*  list,void* obj,int obj_size,int pos);

//获取元素
void* list_get(const List* list,int pos);
void* list_getlast(const List* list);

int list_getInt(void* obj);
float list_getFloat(void* obj);
short list_getShort(void* obj);
long list_getLong(void* obj);
char* list_getChars(void* obj);
#define list_getStruct(obj,T) ((T*)(obj))


//移除元素
void list_removeObj(List* list,void* obj,void (*cmpFn)(List*,void*,void*));

//根据位置删除元素
void list_remove(List* list,int pos);

//释放列表所有元素内存
void list_empty(List* l);

//释放列表本身及所有元素
void list_release(List* list);

//列表是否包含元素
int list_include(const List* list,void* obj,int obj_size);

//是否包含
int list_contains(const List* list,Node* node);

//列表是否为空
void list_is_empty(const List* list);

int list_indexof(const List* list,Node* node);

void list_reverse(List* list);
void list_merge(List* list,List* l);
#endif
