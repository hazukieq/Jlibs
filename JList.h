#ifndef __JLIST_H_
#define __JLIST_H_

#define jlist_foreach(jlist,execueteFn) \
       for(Node* item=jlist->next;item;item=item->next)\
            if(item->obj!=NULL) executeFn(item->obj);

//请注意 node 是一个节点，不是实际数据！
#define jlist_for(jlist,node) \
        for(Node* node=jlist->next;node;node=node->next)

#define jlist2(jlist) \
        for(Node* it=jlist->next;it;it=it->next)

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
} JList;


/**
 * 列表初始化
 * JListType 枚举
 * size 数据类型大小
 * return 返回一个结构体
 */
JList* jlist_init(char ltype);

//根据索引插入元素
void jlist_insert(JList* jlist,void* obj,int obj_size,int pos);

//在尾部添加元素
void jlist_add(JList* jlist,void* obj,int obj_size);


//修改元素值
//void jlist_setObj(JList* jlist,void* obj,int obj_size);
void jlist_set(JList*  jlist,void* obj,int obj_size,int pos);

//获取元素
void* jlist_get(const JList* jlist,int pos);
void* jlist_getlast(const JList* jlist);

int jlist_getInt(void* obj);
float jlist_getFloat(void* obj);
short jlist_getShort(void* obj);
long jlist_getLong(void* obj);
char* jlist_getChars(void* obj);
#define jlist_getStruct(obj,T) ((T*)(obj))


//移除元素
void jlist_removeObj(JList* jlist,void* obj,void (*cmpFn)(JList*,void*,void*));

//根据位置删除元素
void jlist_remove(JList* jlist,int pos);

//释放列表所有元素内存
void jlist_empty(JList* l);

//释放列表本身及所有元素
void jlist_release(JList* jlist);

//列表是否包含元素
int jlist_include(const JList* jlist,void* obj,int obj_size);

//是否包含
int jlist_contains(const JList* jlist,Node* node);

//列表是否为空
void jlist_is_empty(const JList* jlist);

int jlist_indexof(const JList* jlist,Node* node);

void jlist_reverse(JList* jlist);
void jlist_merge(JList* jlist,JList* l);

void jlist_test();
#endif
