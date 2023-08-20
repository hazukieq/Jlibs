#ifndef _JSha256_H__
#define _JSha256_H__
 
#include<string.h>
#include<stdio.h>
#include<stdint.h>

//储存SHA256加密数据
struct Sha256Context{
	char hash[32];//二进制数据
	char bin[257];//二进制字符数据
	char hex[65];//十六进制字符数据
	char raw[];//待加密字符串
};

typedef struct Sha256Context shacontext;

 //哈希加密函数
void jsha_hash(const char *data, size_t len, unsigned char** outptr_ptr);

//获取哈希加密存储结构
void jsha_getobj(const char *data, size_t len,shacontext** contextptr_ptr);
//打印哈希数据存储结构
void jsha_printobj(shacontext* context);
char* jsha_getjson(shacontext* context);

//获取哈希二进制字符串
char* jsha_getbin(const unsigned char* hash,char split_tag);
//获取哈希十六进制字符串
char* jsha_gethex(const unsigned char* hash,char split_tag);

//打印
void jsha_print(const unsigned char* out,int fmt_mode,char* split_tag);

//测试函数
void jsha_test();
#endif
