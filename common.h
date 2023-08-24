#ifndef __COMMON_H__
#define __COMMON_H__

#define FALSE 0
#define TRUE 1

#define Err(msg,var,format) printf("\033[0;31mError on File(%s)_Func<%s>(Line %d)=> " "variable<" #var ">=\"%" #format "\" %s\033[0m\n",__FILE__,__FUNCTION__,__LINE__,var,msg)

#define loge(fmt,args...) printf("Loge on File(%s)_Func<%s>(Line %d)=> \033[0;31m" fmt "\033[0m\n",__FILE__,__func__,__LINE__,##args)

#define logc(fmt,args...) printf("Logc on File(%s)_Func<%s>(Line %d)=> \033[0;35m" fmt "\033[0m\n",__FILE__,__func__,__LINE__,##args)

#define logi(var,format,fmt,args...) printf("Logi on File(%s)_Func<%s>(Line %d)=> \033[0;32mvariable<" #var ">=\"%" #format "\"" fmt "\033[0m\n",__FILE__,__func__,__LINE__,var,##args)


//计算类型内存大小
#define _CH(s) sizeof(char)*(s)
#define _IN(s) sizeof(int)*(s)

//获取转换指针
#define vtr(type,var,val) type* var=(type*) val

//根据结构体成员得到结构体指针
#define ptr(value,_struct,_member)((_struct *)(((unsigned long)&(value))-(unsigned long)(&((_struct *)0)->_member)))


#endif
