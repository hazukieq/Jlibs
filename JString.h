#ifndef __JSTRING_H_
#define __JSTRING_H_
#include "JList.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#define __ARRAY_EXPAND(type,...) (type[]){__VA_ARGS__},sizeof((type[]){__VA_ARGS__})/sizeof(type)
#define __regexs(...) (const char*[]){__VA_ARGS__},sizeof((const char*[]){__VA_ARGS__})/sizeof(char*)

/**
 * const JStr texts,int* resultlen,...(const char** signal_arr,int signal_arr_len)
 */
#define jstr_slit(texts,resultlen,...) jstr_slits(texts,__regexs(__VA_ARGS__),resultlen)
/**
 * texts 字符串
 * new_jc 新字符串
 * ...  多个旧字符串
 */
#define jstr_replace(texts,new_jc,...) jstr_replaces(texts,__regexs(__VA_ARGS__),new_jc)

#define jstr_merge(...) jstr_merges(__regexs(__VA_ARGS__))

/**
 * 自动释放所有JStr实例内存
 * ...: 实例化变量 
 */
#define jstr_auto(...) jstr_freeAll(__ARRAY_EXPAND(JStr,__VA_ARGS__))

struct __str{
        //判断是否属于字符串
        char isJstr[1];

        //记录str_array已使用的量
        int len;

        //记录str_array未使用的量
        int free;

        //字节数组，用于保存字符串
        char buf[];
};

typedef struct __str JString;
typedef char *JStr;
typedef int Boolean;

//internal functions
static void KmpNext(const char* p,int* next);

static int* __str_kmp_search(char* texts,int textslen,const char* signals,int siglen,int* matches);

//获取数字的绝对值
static int __str_abs(int nums);
//获取数字的位数
static int __str_getdigits(int nums);

static void __str_addEndMark(char* cptr,int index);

static int __isFullSize(JStr jc, int applyMem);

static int __str_getFree(JStr jc,int applyMem);

static int __str_getActualSize(JStr jc);

static JStr __str_onBaseMakeMem(JStr jc,int applySize);
static JStr __str_overwriteMakeMem(JStr jc,int applySize);

//根据算法来分配字符串内存，为拼接拓展等预留空间，其申请空间大小记录在`free`属性中
static void* __str_memcpy(void* dest,int dest_size,const void* src,int src_size);

static void __utfobj_init(List* list,char c[],int index, int clen);
static void __addUtfobj(List* list,char c[],int clen,int index,int bits);
static List* __utf_getUtfobjs(JStr jc,int jclen);

static JStr __ansi_subs(JStr jc,int jclen,int start,int end);
static JStr __utf_subs(JStr jc,int jclen,int start,int end);

static JStr __ansi_reverse(JStr jc,int jclen,int start,int end);
static JStr __utf_reverse(JStr jc,int jclen,int start,int end);

static Boolean __str_equal(const char* s1,int s1len,const char* s2,int s2len);
static Boolean __equal(const char* s1,int s1len,const char* s2,int s2len);

static Boolean __str_starts(const JStr jc,const char* signals,int offset);
static Boolean __str_ends(const JStr jc,int jclen,const char* signals,int offset);

//实现难度比较大的函数
static JStr __str_replace(JStr origin,int originlen,const char** old_jcs,int old_jcs_len,const char* new_jc);
static JStr __str_displace(JStr origin,int originlen,const char* old_jc,const char* new_jc);
static void __str_cats(JStr* old_jc,int old_len,const void* t,int len);
static JStr __str_merge(const char** carrs,int len);

static JStr __str_reset(JStr jc,const char* t);
static JStr __str_insert(JStr jc,const char* insertJc,int startIndex);
static JStr* __str_splits(const JStr texts,int textslen,const char* signals,int* resultlen);

//数字方面函数
static int __str_checkEndianfmt();
static JStr __str_int2bin(int n);
static int __str_bin2int(char* chars);

static int __str_tointlen(const void* jc,int size);
static JStr __str_tostr(int nums);

//public methods

//create new String object method
JStr jstr_newlen(const void* str,int len);
JStr jstr_new(const char* str);
JStr jstr_newempty();
JStr jstr_autofit(JStr jc);

int jstr_len(const JStr jc);

/**
 * 注意不能直接赋值给origin,如 origin=jstr_reset(origin...),否则会出现内存泄漏!!!
 */
JStr jstr_reset(JStr old_jc,const char* t);
JStr jstr_copy(const JStr jc);
int jstr_isNone(const JStr jc);
char jstr_charAt(const JStr jc,int index);

/*--截取字符方法,不会破坏原字符--*/
JStr jstr_subAll(const JStr jc,int startIndex);
JStr jstr_subs(const JStr jc,int startIndex, int endIndex);
JStr jstr_subsAnsi(const JStr jc,int startIndex, int endIndex);

//翻转字符
JStr jstr_reverse(const JStr jc);
JStr jstr_subverse(const JStr jc,int start,int end);

//判断是否等价
int jstr_equal(const JStr jc1,const JStr jc2);


int jstr_starts(const JStr jc,const char* signals,int offset);
int jstr_startsWith(const JStr jc,const char* signals);

int jstr_ends(const JStr jc,const char* signals,int offset);
int jstr_endsWith(const JStr jc,const char* signals);


int jstr_indexOf(const JStr origin,const char* signals);
int jstr_lastIndexOf(const JStr origin,const char* signals);
int jstr_contains(const JStr origin,const char* signals);

/*--高难度函数--*/
/**
 * 注意不能直接赋值给origin,如 origin=jstr_displace(origin...),否则会出现内存泄漏!!!
 */
JStr jstr_displaces(JStr origin,const char* old_jc,const char* new_jc);

/**
 * 注意不能直接赋值给origin,如 origin=jstr_replaces(origin...),否则会出现内存泄漏!!!
 * 建议调用 jstr_replace 宏!!
 */
JStr jstr_replaces(JStr origin,const char** old_jcs,int old_jcs_len,const char* new_jc);

/**
 * 注意不能直接赋值给origin,如 origin=jstr_insert(origin...),否则会出现内存泄漏!!!
 */
JStr jstr_insert(JStr jc,const char* insertJc,int startIndex);

/**
 * 注意不能直接赋值给origin,如 origin=jstr_cat(origin...),否则会出现内存泄漏!!!
 */
void jstr_cat(JStr* jc1,const char* jc2);

JStr jstr_merges(const char** carrs,int len);

/*--字符切割--*/
JStr* jstr_splits(const JStr texts,const char* signals,int* resultlen);
JStr* jstr_slits(const JStr texts,const char** signal_arr,int signal_arr_len,int* resultlen);

void jstr_free(JStr jc);
void jstr_frees(JStr* jarrs,int jarrslen);
//自动释放所有JStr实例内存,建议调用宏变量jstr_auto
void jstr_freeAll(JStr* alls,int total);

/*--字符转数字--*/
int jstr_str2num(const JStr jc);
int jstr_str2numlen(const char* jc,int size);
JStr jstr_num2str(int nums);
int jstr_isnum(const JStr jc);
int jstr_isnumlen(const char* jc,int len);

/**--数字转二进制--*/
JStr jstr_int2bin(int number);
JStr jstr_binarrs2int(char* number_chars);
int jstr_bin2int(char* number_chars);
JStr jstr_slicadd(JStr jc,int n,char* addTag);

/**---数字转十六进制--*/
static JStr __str_int2hex(int n);
JStr jstr_int2hex(int n);

/**---字符串转二进制---*/
static JStr __str_str2bin(const char* jc,int ispad,char pad_tag);
JStr jstr_str2bin(const char* jc,int ispad,char pad_tag);

/*---测试方法---*/
void jstr_test();
#endif
