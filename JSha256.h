#ifndef _JSha256_H__
#define _JSha256_H__
 
#include<string.h>
#include<stdio.h>
#include<stdint.h>
struct Sha256Context{
	char hash[32];
	char bin[257];
	char hex[65];
	char raw[];
};

typedef struct Sha256Context shacontext;

 
void jsha_hash(const char *data, size_t len, unsigned char** outptr_ptr);

void jsha_getobj(const char *data, size_t len,shacontext** contextptr_ptr);
void jsha_printobj(shacontext* context);

void jsha_getbin(const unsigned char* hash,char* split_tag,char** binptr_ptr);
void jsha_gethex(const unsigned char* hash,char* split_tag,char** hexptr_ptr);

void jsha_print(const unsigned char* out,int fmt_mode,char* split_tag);
#endif
