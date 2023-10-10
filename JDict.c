#include "JDict.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "JSha256.h"
#include <stdlib.h>

#define CH(x) ((char*)(x))
#define T(x,type) ((type)(x))
#define LOAD_FACTOR(x) ((x)->lfactor_fn((x)->size,(x)->capacity))
#define LOOP(_i,start,end) for(int _i=start;_i<end;_i++)

static unsigned int __hash_fn(void* key){
	unsigned int hash=0;
	
	unsigned char* out;
	char* k=CH(key);
	jsha_hash(k,strlen(k), &out);
	hash = *((unsigned int*)out);
	if(out) free(out);

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
		return;
	}
	
	//printf("realloc memory successfully(%d)\n",new_mem);
	new_dict->hash_fn=__hash_fn;
	new_dict->lfactor_fn=__loadfactor;
	new_dict->capacity=new_mem;
	new_dict->size=0;
	new_dict->sizemask=new_mem-1;	
	for(int i=0;i<new_dict->capacity;i++) 
		new_dict->table[i]=_dhead_init();
	
	int len=0;
	dnode** entries=jdict_entries(dict,&len);
	if(len>0){
		for(int i=0;i<len;i++){
			jdict_set(&new_dict,entries[i]->key,entries[i]->val);
		}
		new_dict->size=len;
		printf("used_size=>%d\n",new_dict->size);
	}

	jdict_entries_free(entries,len);
	jdict_release(*jdict);
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
			free(node);
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

	for(int i=0;i<dict->capacity;i++)
		dict->table[i]=_dhead_init();

	return dict;
}

void jdict_set(JDict** jdict,void* key,void* value){
	if(jdict==NULL&&*jdict==NULL) return;
	
	double loadf=LOAD_FACTOR(*jdict);
	double LOAD_FACTOR_MAX=0.75;
	if(loadf>LOAD_FACTOR_MAX){
		//printf("\033[0;31mextended memory...loadf:%f\n\033[0m",loadf);
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
	if(dict==NULL||dict->size==0) return NULL;
	
	void** keys=malloc(dict->size*sizeof(void*));
	if(keys==NULL||keys_len==NULL) return NULL;

	int len=0;
	DICT_CYCLE(dict,keys[len++]=cur->key;cur=cur->next;);
	*keys_len=len;
	return keys;
}

void jdict_keys_free(void **keys){
	if(keys) free(keys);
}

dnode** jdict_entries(JDict* dict,int* entries_len){
	if(dict==NULL||dict->size==0) return NULL;
	int mlen=dict->size;
	dnode** entries=malloc(sizeof(dnode*)*mlen);
	if(entries==NULL) return NULL;
	if(entries_len==NULL) return NULL;

	int len=0;
	LOOP(i,0,dict->capacity){
		if(dict->table[i]){
			dnode* cur=dict->table[i]->next;
			while(cur){
				if(mlen>len){
					int tmp=mlen*2;
					dnode** nentries=malloc(sizeof(dnode*)*tmp);
					if(nentries==NULL){
						LOOP(h,0,mlen) free(entries[h]);
						free(entries);
						return NULL;
					}
					LOOP(i,0,mlen){
						if(entries[i]) nentries[i]=entries[i];
					}
					free(entries);
					entries=nentries;
				}
				
				entries[len]=malloc(sizeof(dnode));
				if(entries[len]==NULL){
					LOOP(k,0,mlen) free(entries[k]);
					free(entries);
					return NULL;
				}
				memcpy(entries[len++],cur,sizeof(dnode));
				cur=cur->next;
			}
		}
	}
		
	*entries_len=len;
	return entries;
}

void jdict_entries_free(dnode** entries, int entries_len){
	if(entries_len==0) return;
	LOOP(k,0,entries_len){
		if(entries) free(entries[k]);
	}
	free(entries);
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
					//printf("\033[0;31mmatched <%s,%s> with key<%s>\n\033[0m",
					//		CH(cur->key),CH(cur->val),CH(key));
					
					if(cur->next){
						memmove(cur,cur->next,sizeof(dnode));
						free(cur->next);
					}else{
						free(cur);
						dict->table[i]->next=NULL;
					}
					dict->size--;
					return;
				}
				cur=cur->next;
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
	jdict_keys_free(keys);
}
void jdict_empty(JDict* dict){
	if(dict==NULL) return;
	for(int i=0;i<dict->capacity;i++){
		if(dict->table[i]){
			dnode* cur=dict->table[i]->next;
			while(cur){
				dnode* next=cur->next;
				free(cur);
				cur=next;
				if(next==NULL) break;
			}
			free(dict->table[i]);
		}
	}
}


void jdict_release(JDict* dict){
	if(dict==NULL) return;
	jdict_empty(dict);
	if(dict) free(dict);
}


void jdict_test(){
	//--PASS jdict_init--/
	//--PASS jdict_release--/
	//--PASS jdict_empty--/
	//--PASS jdict_set--/
	//--PASS jdict_del--/
	//--PASS jdict_remove--/
	//--PASS _extend--/
	//--PASS jdict_entries--/
	//--PASS jdict_keys--/
	JDict* ldict=jdict_init();
	jdict_set(&ldict,"hello","2593753494");
	jdict_set(&ldict,"hello","hazukie erii how are you?");
	jdict_set(&ldict,"khello","world");
	jdict_set(&ldict,"lhello","world");
	jdict_set(&ldict,"world","this is a another thing.");
	jdict_del(ldict, "hello");	
	jdict_remove(ldict,"world");

	jdict_info(ldict);
	jdict_print(ldict);

	jdict_release(ldict);


	//--PASS jdict_keys&jdict_entries--/	
	JDict* kdict=jdict_init();
	int klen=0;	
	void** keys=jdict_keys(kdict,&klen);
	LOOP(k,0,klen) printf("key->%s\n",(char*)keys[k]);
	free(keys);
	int elen=0;
	dnode** entries=jdict_entries(kdict,&elen);
	printf("len->%d\n",elen);
	if(entries){
		LOOP(j,0,elen){
			printf("entries[%d]: dnode<%s,%s>\n",j,(char*)entries[j]->key,(char*)entries[j]->val);
		}
	}
	jdict_entries_free(entries, 1);
	jdict_release(kdict);
	//--jdict_entries&jdict_entries--/
	
		
	//--PASS jdict_for--/	
	//--PASS jdict_setbyVal--/
	//--PASS jdict_print--/
	//--PASS jdict_get--/
	//--PASS jdict_clear--/
	//
	JDict* dict=jdict_init();
	char lkey[12];
	for(int i=0;i<100;i++){
		sprintf(lkey,"%di",i);
		jdict_set(&dict,lkey,"alova hello");
	}

	jdict_for(dict,cur){
		if(cur) printf("\033[0;32mfor_loop: <%s,%s>\n\033[0m",(char*)cur->key,(char*)cur->val);
	}

	jdict_setbyVal(dict,"alova hello","alova q'hello");
	jdict_print(dict);	
	
	printf("used_size:%d,capacity:%d\n",dict->size,dict->capacity);
	
	void* val=jdict_get(dict,"world");
	if(val) printf("val: \033[0;31m%s\n\033[0m",CH(val));

	jdict_clear(&dict);
	jdict_set(&dict,"world","hahahha,this is another new hash_dict dayo~");
	jdict_print(dict);
	jdict_release(dict);
}


int main(void){
 	jdict_test();
	return 0;
 }

