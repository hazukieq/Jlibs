#include "Any.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "common.h"

int main(void){
	char* s="hello";
	Any any={10,6,0x01,s};
	Any* anyp=anyPtr(any.val);
	printf("%d,%d,%s\n",anyp->capacity,anyp->size,
			(char*)anyp->val);
	return 0;
}

