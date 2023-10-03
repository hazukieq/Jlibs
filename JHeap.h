#ifndef _JHEAP_H__
#define _JHEAP_H__
#include "Any.h"

struct _HTree{
	int capacity;
	int size;
	Any* arr[];
};

typedef struct _HTree JHeap;

JHeap* jheap_init(int capacity);

Any* jheap_pop(JHeap* jh);

void jheap_push_any(JHeap* jh,Any* any);
void jheap_push(JHeap* jh,void* val,int size);

Any* jheap_peak(JHeap* jh);

int jheap_size(JHeap* jh);

int jheap_isNone(JHeap* jh);

void jheap_release(JHeap* jh);

#endif
