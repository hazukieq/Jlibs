#include "JDict.h"

#define CH(x) ((char*)(x))
#define T(x,type) ((type)(x))
#define LOAD_FACTOR(x) ((x)->lfactor_fn((x)->size,(x)->capacity))
static unsigned int __hash_fn(void* key){
	unsigned int hash=0;
	
	unsigned char* out;
	char* k=CH(key);
	jsha_hash(k,strlen(k), &out);
	hash = *((unsigned int*)out);

	return hash;
}

static double __loadfactor(int size,int capacity){
	return (double)size/capacity;
}

//覆盖式申请内存
static void _extend(JDict** jdict){
	JDict* dict=*jdict;

	int new_mem=dict->capacity*2;
	JDict* new_dict=(JDict*)malloc(sizeof(JDict)+sizeof(dhead*)*new_mem);
	if(new_dict==NULL){
		printf("malloc failed\n");
	}else{
		printf("realloc memory successfully(%d)\n",new_mem);
		new_dict->hash_fn=__hash_fn;
		new_dict->lfactor_fn=__loadfactor;
		new_dict->capacity=new_mem;
		new_dict->size=0;
		new_dict->sizemask=new_mem-1;
		
		dhead** table=malloc(sizeof(dhead*)*new_mem);
		if(table==NULL) return;
		memcpy(new_dict->table,table,sizeof(dhead*)*new_mem);	
		for(int i=0;i<new_dict->capacity;i++) 
			new_dict->table[i]=_dhead_init();
		free(table);
		int len=0;
		dnode** entries=jdict_entries(dict,&len);
		if(len>0){
			for(int i=0;i<len;i++)
				jdict_set(&new_dict,entries[i]->key,entries[i]->val);
			new_dict->size=len;
			printf("used_size=>%d\n",new_dict->size);
		}
	}
	*jdict=new_dict;
}


static dhead* _dhead_init(){
	dhead* head=malloc(sizeof(dhead));
	if(head==NULL){
		printf("malloc for dhead failed\n");
		return NULL;
	}
	
	head->len=0;
	head->next=NULL;
	return head;
}

//创建一个节点
static dnode* _create_dnode(void* key,void* val){
	dnode* node=(dnode*) malloc(sizeof(dnode));
	if(node==NULL){
		printf("create dnode failed\n");
		return NULL;
	}

	node->key=key; 
	node->val=val;
	node->next=NULL;
	return node;
}

//将节点添加到表头中去,更新链表头的指针和哈希数组信息
static void _update_dhead(JDict* dict,dhead* head,dnode* node){
	if(head==NULL||node==NULL) return;
	
	dnode* current=head->next;
	if(current==NULL){
		head->next=node;
		dict->size++;
		return;
	}
	while(current){
		if(dict->hash_fn(node->key)==dict->hash_fn(current->key)){
			current->val=node->val;
			return;
		}
		current=current->next;
	}
	node->next=head->next;
	head->next=node;
	dict->size++;
}

#define DEFAULT_SIZE 53
JDict* jdict_init(){
	JDict* dict=(JDict*)malloc(sizeof(JDict)+sizeof(dhead*)*DEFAULT_SIZE);
	if(dict==NULL) return NULL;

	dict->hash_fn=__hash_fn;
	dict->lfactor_fn=__loadfactor;
	dict->size=0;
	dict->capacity=53;
	dict->sizemask=dict->capacity-1;

	dhead** table=malloc(sizeof(dhead*)*DEFAULT_SIZE);
	if(table==NULL) return NULL;
	memcpy(dict->table,table,sizeof(dhead*)*DEFAULT_SIZE);	
	for(int i=0;i<dict->capacity;i++)
		dict->table[i]=_dhead_init();
	free(table);

	return dict;
}

void jdict_set(JDict** jdict,void* key,void* value){
	if(jdict==NULL&&*jdict==NULL) return;
	
	double loadf=LOAD_FACTOR(*jdict);
	double LOAD_FACTOR_MAX=0.75;
	if(loadf>LOAD_FACTOR_MAX){
		printf("\033[0;31mextended memory...loadf:%f\n\033[0m",loadf);
		_extend(jdict);
	}
	JDict* dict=*jdict;
	
	int index=dict->hash_fn(key)%dict->sizemask;	
	dhead* head=dict->table[index];
	dnode* node=_create_dnode(key,value);
	_update_dhead(dict,head,node);
}

void* jdict_get(JDict* dict,void* key){
	if(dict==NULL) return NULL;
	
	unsigned int key_hash=dict->hash_fn(key);
	int index=key_hash%dict->sizemask;
	
	dhead* head=dict->table[index];
	dnode* cur=head->next;
	if(cur==NULL) return NULL;
	while(dict->hash_fn(cur->key)!=key_hash){
		if(cur!=NULL)
			cur=cur->next;
		else return NULL;
 	}
	return cur->val;
}




#define DICT_CYCLE(d,logics) for(int i=0;i<d->capacity;i++){\
						if(d->table[i]){\
							dnode* cur=d->table[i]->next;\
							while(cur){\
							   logics\
							}\
						}\
					}

void** jdict_keys(JDict* dict,int* keys_len){
	void** keys=malloc(dict->size*sizeof(void*));
	if(keys==NULL||keys_len==NULL) return NULL;

	int len=0;
	DICT_CYCLE(dict,keys[len++]=cur->key;cur=cur->next;);
	*keys_len=len;
	return keys;
}

dnode** jdict_entries(JDict* dict,int* entries_len){
	dnode** entries=malloc(sizeof(dnode*)*dict->size);
	if(entries==NULL||entries_len==NULL) return NULL;

	int len=0;
	DICT_CYCLE(dict,entries[len++]=cur;cur=cur->next;);
	*entries_len=len;
	return entries;
}

void jdict_setbyVal(JDict* dict,void* old_value,void* new_value){
	DICT_CYCLE(dict,if(CH(cur->val)==old_value)cur->val=new_value;cur=cur->next;);
}

void jdict_cat(JDict* dict,JDict** dict1){
	DICT_CYCLE(dict,jdict_set(dict1,cur->key,cur->val);cur=cur->next;);
}

void jdict_del(JDict* dict,void* key){
	for(int i=0;i<dict->capacity;i++){
		if(dict->table[i]){
			dnode* cur=dict->table[i]->next;
			while(cur){
				if(__hash_fn(cur->key)==__hash_fn(key)){
					//printf("\033[0;32mfinded %s\n\033[0m",CH(cur->key));
					if(cur->next) dict->table[i]->next=cur->next;
					else dict->table[i]->next=NULL;

					free(cur);
					dict->size--;
					return;
				}
				dnode* next=cur->next;
				if(next&&__hash_fn(next->key)==__hash_fn(key)){
					cur->next=next->next;
					//printf("\033[0;31mfinded %s\n\033[0m",CH(next->key));
					free(next);
					dict->size--;
					return;
				}else cur=cur->next;
			}
		}
	}
}

void jdict_remove(JDict* dict,void* value){
	int max=100;
	void** keys=malloc(max*sizeof(void*));
	if(keys==NULL) return;
	
	int len=0;
	jdict_for(dict,cur){
		if(len==max){
			max*=2;
			void* nkeys=realloc(keys,sizeof(void*)*max);
			if(nkeys==NULL) return;
			printf("extended memory successfully in remove_fn(%d)...\n",max);
			keys=nkeys;
		}
		if(CH(cur->val)==CH(value))
			keys[len++]=cur->key;
	}
	
	for(int j=0;j<len;j++)
		jdict_del(dict,keys[j]);
	free(keys);
}

void jdict_clear(JDict** dict){
	if(dict==NULL) return;
	int keys_len=0;
	void** keys=jdict_keys(*dict,&keys_len);
	if(keys&&keys_len>0){
		while(keys&&keys_len--)
			jdict_del(*dict,keys[keys_len]);
	}

}

void jdict_release(JDict** dict){
	if(dict) return;
	jdict_clear(dict);
	
	if(*dict) free(*dict);
}


int main(void){
	JDict* dict=jdict_init();
	jdict_set(&dict,"hello","2593753494");
	jdict_set(&dict,"hello","world");
	jdict_set(&dict,"hello","加油！");
	jdict_set(&dict,"world","this is a another thing.");
	jdict_del(dict, "hello");

	for(int i=0;i<1200;i++){
		char* key=malloc(12);
		sprintf(key,"%di",i);
		jdict_set(&dict,key,"hello");
	}
	printf("loadfactor:%f\n",dict->lfactor_fn(dict->size,dict->capacity));
	
	
	jdict_remove(dict,"hello");
	
	for(int i=0;i<100;i++){
		char* key=malloc(12);
		sprintf(key,"%di",i);
		jdict_set(&dict,key,"alova hello");
	}

	jdict_for(dict,cur)
		printf("\033[0;32mfor_loop: <%s,%s>\n\033[0m",(char*)cur->key,(char*)cur->val);

	jdict_setbyVal(dict,"alova hello","alova q'hello");
	jdict_print(dict);	
	
	int entries_len=0;
	dnode** entries=jdict_entries(dict, &entries_len);
	for(int j=0;j<entries_len;j++) printf("%d: key,val=>Entry<%s,%s>\n",
			j+1,(char*)entries[j]->key,(char*)entries[j]->val);
	free(entries);
	
	int keylens=0;
	void** keys=jdict_keys(dict,&keylens);
	for(int i=0;i<keylens;i++)printf("%d:Key<%s>\n",i+1,(char*)keys[i]);
	free(keys);

	printf("used_size:%d,capacity:%d\n",dict->size,dict->capacity);
	
	void* val=jdict_get(dict,"world");
	if(val) printf("val: \033[0;31m%s\n\033[0m",CH(val));

	jdict_clear(&dict);
	jdict_set(&dict,"world","hahahha,this is another new hash_dict dayo~");
	jdict_print(dict);
	jdict_release(&dict);
	return 0;
}

