#ifndef _JDICT_H__
#define _JDICT_H__

//dict 包含table    ->哈希字典
//table 包含dhead   ->数组
//dhead 由dnode链接 ->链表
struct _dnode{
	struct _dnode* next;
	void* val;//值
	void* key;//键
};
typedef struct _dnode dnode;

struct _dhead{
	int len;//已存储的键值对数量
	dnode* next;//下个节点指针
};
typedef struct _dhead dhead;

struct _dict{
	unsigned int (*hash_fn)(void* key);
	double (*lfactor_fn)(int size,int capacity); //负载因子
	int capacity;//table分配的总空间 桶总数量
	int size;//table已用空间 已用的键值对总数量
	int sizemask;
	dhead* table[];
};
typedef struct _dict JDict;

static unsigned int __hash_fn(void* key);
static void _extend(JDict** dict);//在原来基础上增加内存
static dhead* _dhead_init();

static dnode* _create_dnode(void* key,void* val);//创建一个节点
static void _update_dhead(JDict* dict,dhead* head,dnode* node);//将节点添加到表头中


//初始化
JDict* jdict_init();

void jdict_set(JDict** jdict,void* key,void* value);
void* jdict_get(JDict* dict,void* key);

//记得手动释放内存!!!
void** jdict_keys(JDict* dict,int* keys_len);
void jdict_keys_free(void** keys);
//记得手动释放内存!!!
dnode** jdict_entries(JDict* dict,int* entries_len);
void jdict_entries_free(dnode** entries,int entries_len);

#define jdict_for(dict,cur) for(int i=0;i<dict->capacity;i++)\
				      for(dnode* cur=dict->table[i]->next;cur;cur=cur->next)

#define jdict_print(dict) for(int i=0;i<dict->capacity;i++){\
				      for(dnode* cur=dict->table[i]->next;cur;cur=cur->next)\
					printf("\033[0;32m%s[%d]: <%s,%s>\n\033[0m",#dict,i,(char*)cur->key,(char*)cur->val);\
			}

#define jdict_info(dict) printf("\033[0;32m%s=>[loadfactor:%f,capacity:%d,size:%d]\033[0m\n",#dict,dict->lfactor_fn(dict->size,dict->capacity),dict->capacity,dict->size);


//将字典中所有相同的值都修改为其他值
void jdict_setbyVal(JDict* dict,void* old_value,void* new_value);

//将字典1和字典2合併,即在字典1后面添加字典2
void jdict_cat(JDict* dict,JDict** dict1);

//根据键移除节点
void jdict_del(JDict* dict,void* key);

//根据值移除节点
void jdict_remove(JDict* dict,void* value);

//清空表和链表
void jdict_clear(JDict** dict);

//释放哈希表内存
void jdict_empty(JDict* dict);
void jdict_release(JDict* dict);

//测试函数
void jdict_test();
#endif

