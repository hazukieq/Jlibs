#include "JString.h"
#include <stdlib.h>
/**---内部宏---*/
//输出红色警告信息
//int转char
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

//根据结构成员指针地址找到结构体首位地址
//因为结构体的内存是动态分配的，所以我们可以可以通过结构体成员buf减去结构体占用的空间大小来得到结构体的首位地址，即结构体指针
#define jptr(name) ((JString*)(name-(sizeof(JString)))) 
#define vjptr(name,val) JString* val=jptr(name)

//检查是否属于JString类型
#define assertJc(name,ret) if(*(jptr(name)->isJstr)!='1'){\
                                            Err("is not a JString object,please initialize it by jstr_newlen() or jstr_new().",name,s);\
                                           return ret;\
}


/*
 * 内部方法的参数全都假定不为空！！！如果出错则肯定是公共方法调用内部方法时传入了含空的参数！！！
 */

//经典kmp算法
static void KmpNext(const char* p,int* next){
        int pLen=strlen(p);
        int k=-1;
        int j=0;
        next[0]=-1;
        //最后一位不需要算
        while(j<pLen-1){
                if(k==-1||p[j]==p[k]){
                        j++,k++;

                        if(next[j]!=next[k])
                                next[j]=k;
                        else next[j]=next[k];
                        //logc("before: j<%d>,k<%d> =>j<%d>,k<%d>,next[%d]=<%d>",j-1,k-1,j,k,j,k);
                }else {
                        //logc("\tj<%d>,k<%d> => %d,%d",j,k,j,next[k]);
                        k=next[k];
                }
        }
}

static int* __str_kmp_search(char* texts,int textslen,const char* signals,int siglen,int* matches){
        int initialSize=10;
        //记录成功配对字符下标位置的数组
        int* matchedNums=NULL;
        matchedNums=malloc(_IN(initialSize));
        if(matchedNums==NULL) return NULL;
        
        //kmp算法中子串映射的next数组
        int* prefixs=malloc(_IN(siglen));
        if(prefixs==NULL) return NULL;

        KmpNext(signals,prefixs);

        int i=0,j=0,matched=0;
        while(i<textslen&&j<siglen){
                //内存不够用时,需要扩展
                if(matched+1>initialSize){
                        //logc("调整内存<%d>至<%d>,matched<%d>",initialSize,initialSize*2,matched);
                        int* tmp=NULL;
                        initialSize *=2;
                        tmp=realloc(matchedNums,_IN(initialSize));
                        if(tmp==NULL) goto clear;
                        matchedNums=tmp;
                        //logc("reallocate memory successfully."); 
                }
                //当j等于子串长度时且最后主串字符和子串字符相等时,说明配对成功
                //此时需要记录下当前配对字符的下标位置,并将其传入配对成功数组中去
                if(j==siglen-1&&texts[i]==signals[j]){
                        //logc("子串匹配于母串起始位置：%d\n",i-j);
                        matchedNums[matched]=(i-j);
                        matched++;
                        j=prefixs[j];
                }
                //kmp算法核心,不匹配时会动态回溯上一次配对成功的位置
                if(j==-1||texts[i]==signals[j]){
                        i++;
                        j++;
                }else j=prefixs[j];
        }

        free(prefixs);
        if(matches==NULL) return NULL;
        *matches=matched;
        return matchedNums;
clear:
        loge("error...");
        free(prefixs);
        free(matchedNums);
        return NULL;
}

//绝对值
static int __str_abs(int nums){
        if(nums<0) return ~nums+1;
        return nums;
}
//数字位数
static int __str_getdigits(int nums){
        int digits=0;
        if(nums==0) return 1;
        while(nums){
                nums=nums/10;
                digits++;
        }

        return digits;
}

//注意字符串内存会自动分配额外的一个字节来储存\0标识
static void __str_addEndMark(char* cptr,int index){
        *(cptr+index) ='\0';
}

//JString结构体的分配到的实际内存大小
static int __str_getActualSize(JStr jc){
        return jptr(jc)->len+jptr(jc)->free+1;
}

//1表示充足,0表示不足
static int __isFullSize(JStr jc, int applyMem){
        if(jptr(jc)->free+jptr(jc)->len>applyMem) return TRUE;
        return FALSE;
}


//此方法仅适用于不改变原有字符内容的情况下，比如合并字符，或者在原字符基础上新增内容
//传入申请的内存大小和真实剩下的可用内存空间比较，看可用空间是否装得下传入申请内存
//如果可以，返回装下后的剩余空间，否则返回 0
static int __str_getFree(JStr jc,int applyMem){        
        JString* jstr=jptr(jc);
        if(jptr(jc)->free>applyMem)
                return jstr->free-applyMem;
        return 0;
}

/* 
 * 动态分配内存
 * 在原有基础上新增内存
 * realloc函数对结构体的内存扩容时，会有两种情况：
 *      1. 在原结构体后面申请一段空间，此时指针指向的首地址没有变化
 *      2. 若申请空间失败，则会另外开辟一段新的内存空间，并释放掉原结构体空间内存，并返回一个新的内存地址。
 * 故不管上述情况如何，我们都需要一个指针接收realloc返回的内存首地址，进而避免旧内存空间分配不足的问题发生
 * @param jc 字符串
 * @param applySize 预备申请的内存空间
 * @return 一个指向JSring成员buf的ptr
 */
#ifndef __JSTRING_MAX_COPY_VALUE 
#define __JSTRING_MAX_COPY_VALUE (1024*1024)
#endif
static JStr __str_onBaseMakeMem(JStr jc,int applySize){
        vjptr(jc,jstr);
        if(jstr->free>applySize) return jc;
        
         //如果字符长度在1M以下,则每次修改字符时都会多拷贝一份内存,超过1M的话，则复制 1M 多余内存
        int len=jstr->len;
        int newlen=jstr->len+applySize;
        if(newlen<__JSTRING_MAX_COPY_VALUE)
                newlen*=2;
        else
                newlen+=__JSTRING_MAX_COPY_VALUE;
        
        JString* new_jstr=realloc(jstr,_CH(newlen+1)+sizeof(JString));
        if(new_jstr==NULL){
                Err("try to reallocate memory failed",newlen,d);
                return NULL;
        }
        new_jstr->free=newlen-len;
        *new_jstr->isJstr='1';
        return new_jstr->buf;
}

//覆盖式申请内存
static JStr __str_overwriteMakeMem(JStr jc,int overwriteSize){
        if(overwriteSize==__str_getActualSize(jc)) {
                logc("overwriteSize<%d> is equal to actualSize<%d>",overwriteSize,__str_getActualSize(jc));
                return jc;
        }

        JString* jstr=jptr(jc);
        int oldlen=jstr->len+jstr->free;

        JString* new_jstr=realloc(jstr,_CH(overwriteSize+1)+sizeof(JString));
        if(new_jstr==NULL){
                Err("try to realloc memory failed.",overwriteSize,d);
                return NULL;
        }
        int freelen=oldlen>overwriteSize?oldlen-overwriteSize:0;
        new_jstr->free=freelen;
        new_jstr->len=overwriteSize;
        *new_jstr->isJstr='1';

        //logc("size<%d> is updated to <%d>bytes,free<%d>",oldlen,overwriteSize,freelen);
        return new_jstr->buf;
}

/**memory copy method,
 * mode has two options: 
 *      order(1),reverse order(-1)
 */
static void* __str_memcpy(void* dest,int dest_size,const void* src,int src_size){
        /*
         * try to avoid the memory leak 
         * while copying data from source to destination.
         * Commonly the dest variable may be a pointer 
         * that owns uninitialized memory,
         * so makes us to get the unaccurate size of it.
         * We must request the user
         * to set the size of the dest variable directly.
         */ 

        if(dest == NULL||src==NULL) {
                loge("dest<%s> or src<%s> may be NULL",(char*)dest,(char*)src);
                return NULL;
        }
        if(dest_size==0||src_size==0||dest_size<src_size) 
        {
                loge("src_size<%d> is larger than dest_size<%d>",(int)dest_size,(int)src_size);
                return NULL;
        }
         
        //considered the efficiency of copying the data is bad when the size of data i too large.
        //we must use the memcpy to imporve the efficiency.
        void* ret=memcpy(dest, src,_CH(src_size));
        return ret;
}


struct __Utfobj{
        //标识这是有几位字节组成的字符
        int len;//实际字节长度
        int utfindex;//utf编码中的位置
        
        int start;//开始位置
        int end;//结束位置
        char acmps[];//字符存储数组
};


static void __addUtfobj(List* list,const JStr c,int clen,int index,int bits){
                        JStr ne=NULL;
                        int endIndex=index+bits;
                        if(endIndex>clen){
                                loge("found endIndex<%d> is larger than clen<%d>,but ignored it...",endIndex,clen);
                                endIndex=clen; 
                        }
                        ne=__ansi_subs(c,clen,index,endIndex);
                        
                        struct __Utfobj* _u=malloc(sizeof(struct __Utfobj)+_CH(bits+1));
                        _u->len=bits;
                        _u->start=index;
                        _u->end=endIndex;
                        _u->utfindex=list->len;
                        
                        __str_memcpy(_u->acmps,bits,ne,bits);
                        __str_addEndMark(_u->acmps,bits);
                
                        list_add(list,_u,sizeof(struct __Utfobj)+_CH(bits+1));
                        free(_u);
                        jstr_free(ne);
                        ne=NULL;
}


static List* __utf_getUtfobjs(JStr jc,int jclen){
        /* cnt   所有字节的总长度
         * cnt+1 一位数字节,cnt+2 两位数字节,cnt+3 三位数字节
         * cnt+4 四位数字节,cnt+5 五位数字节,cnt+6 六位数字节
         */
        #define _BIT(var,val) var>val
        #define BITZ(v) _BIT(jstr,v)
        #define AOBJ(inum,bits) __addUtfobj(list,jc,jclen,inum,bits)
                
        List* list=list_init(LSTRUCT);
        for(int i=0;i<jclen;i++){
                //convert unsigned binary data.
                unsigned char jstr=jc[i];
                if(BITZ(0xFC)) AOBJ(i,6);
                else if(BITZ(0xF8)) AOBJ(i,5);
                else if(BITZ(0xF0)) AOBJ(i,4);
                else if(BITZ(0xE0)) AOBJ(i,3);
                else if(BITZ(0xC0)) AOBJ(i,2);
                else if((jstr&0x80)==0) AOBJ(i,1);
        }
        return list;
}

/**----以下方法操作失败均返回NULL代码！----**/
/* 
 * positive numbers means a-z order:
 * assume -1 is the end of the string,
 * and 0 is a header of the string.
 * negative numbers means reverse order:
 * assume 0 is the end of the string,
 * -1 is a header of the string.
 *不会破坏原有字符
 */
static JStr __ansi_subs(JStr jc,int jclen,int start,int end){
        //negative index=length of the string - positive index.
        if(start==0&&end==0) {
                loge("start<%d> and end<%d> is zero,so return a empty string",start,end);
                return jstr_newempty();
        }
        if(start<0) {
                if(start==-1) start=jclen-1;
                else start= jclen+start;
        }

        if(end<0){
                if(end==-1) end=jclen;
                else end=jclen+end;
        }

        int size=end>start?end-start:start-end;
        if(size<=0) {
                loge("jclen<%d>,size<%d>is 0,but func is ignored it.(start<%d>,end<%d>)",jclen,size,start,end);
                return jstr_newempty();
        }
        JStr new_jc=jstr_newlen("", size);
        memcpy(new_jc,jc+start,size);
        __str_memcpy(new_jc,size,jc+start,size);
        return new_jc;
}

//不破坏原有字符内容
static JStr __utf_subs(JStr jc,int jclen,int start,int end){
         /*guess the enconding og the characters.
         * a is ANSI char,u is UTF8 char.
         * gbk is g,others is unsupported.
         */
        if(start==0&&end==0) return jstr_newempty();
        List* utfs=__utf_getUtfobjs(jc,jclen);
        int carrs_len=utfs->len;

        if(start>carrs_len||end>carrs_len) { 
                loge("start<%d> or end<%d> maybe is invalid index,maxUtfSize<%d>.\
                                (hints: you must pass utf_indexs!)",start,end,carrs_len);
                return NULL;
        }

        //negative index=length of the string - positive index.
        int specialMode=0;
        if(start<0) {
                //start=-1,end=-1是特殊情况
                //表示截取字符串的最后空白部分
                if(start==-1&&end==-1) {
                        start=carrs_len;
                        specialMode=1;
                }
                else {
                        start= carrs_len+start;
                        specialMode=0;
                }
        }
        
        if(end<0){
                if(end==-1) end=carrs_len;
                else end=carrs_len+end;
        }

        int size=0;
        list_for(utfs,current){        
                struct __Utfobj* obj=(struct __Utfobj*) current->obj;
                if(obj==NULL) goto cleanup;
                if(obj->utfindex>=start&&obj->utfindex<end)
                        size+=obj->len;
        }

        if(size==0) {
                if(specialMode==0) logc("jc<%s>(<%d>) is zero(start<%d>,end<%d>),but ignored it...",jc,size,start,end);
                return jstr_newempty();
        }
       
        //实际开始位置索引并不是utf索引!!!
        int actualSatrt=((struct __Utfobj*)list_get(utfs,start))->start;
        
        JStr new_jc=jstr_newlen("",size);
        __str_memcpy(new_jc,_CH(size),jc+actualSatrt,_CH(size));
        __str_addEndMark(new_jc,size);
        
        list_release(utfs);
        return new_jc;

cleanup:
        if(utfs) list_release(utfs);
        return NULL;
}

//不会破坏原有字符
static JStr __ansi_reverse(JStr jc,int jclen,int start,int end){  
        //调用截取函数对字符截取，然后再进行翻转
        JStr neo_=__ansi_subs(jc,jclen,start,end);
        vjptr(neo_,hr);
        jclen=hr->len;
 
        if(jclen==0||jclen==1) return neo_;
     
        //折中交换法
        for(int i=0;i<jclen/2;i++){ 
                char tmp=*(neo_+i);
                *(neo_+i)=*(neo_+jclen-i-1);
                *(neo_+jclen-i-1)=tmp;
        }
        return neo_;
}

//不会破坏原有字符
static JStr __utf_reverse(JStr jc,int jclen,int start,int end){ 
        JStr neo_=__utf_subs(jc,jclen,start,end);
        JString* hr=jptr(neo_);

        //零个或一个字符时,不用翻转
        if(hr->len==0||hr->len==1) return neo_;

        List* utfs=__utf_getUtfobjs(neo_,hr->len);

        int hrlen=0;
        int utfslen=utfs->len;
        while(utfslen--){
                struct __Utfobj* utf=(struct __Utfobj*)list_get(utfs,utfslen);
                if(utf==NULL) goto clearup;
                for(int i=0;i<=utf->len;i++){
                        char ch=utf->acmps[i];
                        neo_[hrlen+i]=ch;
                }
                hrlen+=utf->len;
        }
        list_release(utfs);
        return neo_;
clearup:
        list_release(utfs);
        return NULL;
}

//不会破坏原有字符
//布尔值比较
static Boolean __str_equal(const char* s1,int s1len,const char* s2,int s2len){
        int check=TRUE;
        while(--s1len>=0){
                if(*(s1+s1len)!=*(s2+s1len)){
                        check=FALSE;
                        break;
                }
        }
        return check;
}

//不会破坏原有字符
//推荐内部函数调用方法，该函数会对传入参数进行判空
static Boolean __equal(const char* s1,int s1len,const char* s2,int s2len){
        if(s1==NULL||s2==NULL) return FALSE;
        if(s1len!=s2len) return FALSE;
        
        return __str_equal(s1,s1len,s2,s2len);
}

//不会破坏原有字符
//布尔值比较
static Boolean __str_starts(const JStr jc,const char* signals,int offset){ 
        /*
         * 匹配字符下标从0开始
         * 待匹配字符下标是从offset(如果有)开始，直到offset+siglen结束
         * 在这一过程循环比较每个字节是否相等，若不等则跳出
         */
        if(offset<0) offset=0;

        int check=TRUE;
        int jc_count=offset;
        int sig_count=0;
        int siglen=strlen(signals);
        while(--siglen>=0){
                if(jc[jc_count++]!=signals[sig_count++]){
                        check=FALSE;
                        break;
                }
        }

        return check;
}

//不会破坏原有字符
//布尔值比较
static Boolean __str_ends(const JStr jc,int jclen,const char* signals,int offset){
        //若匹配字符长度大于待匹配字符,则返回
        //if(strlen(signals)>jptr(jc)->len) return FALSE;//存疑？
        
        //注意signals索引是从0开始的
        //而jc的索引却是start(jc总长度-字符偏移-匹配字符总长度)开始,直到结尾
        //在这一过程循环比较每个字节是否相等,若不等则跳出
        if(offset<0) offset=0;
        //待匹配字符开始下标
        int start=(jclen-offset)-strlen(signals);

        //匹配字符长度
        int siglen=strlen(signals);
        int sig_count=0;
        int check=TRUE;
        
        while(--siglen>=0){
                if(jc[start++]!=signals[sig_count++]){
                        check=FALSE;
                        break;
                }
        }
        return check;
}


//会改变原有字符内容！！
//字符是连串字母时，会出现替换不了的情况 
//复制命中字符前片段到暂存字符串中
//替换命中字符，其开始索引=上一段命中字符前片段结束索引(总长度)+命中字符前片段长度
//暂存字符串补上原文字符后半段
//失败时均返回原来字符,不会修改原来字符！
static JStr __str_displace(JStr origin,int originlen,const char* old_jc,const char* new_jc){
        int oldlen=strlen(old_jc);
        int newlen=strlen(new_jc);
        
        //kmp搜索匹配
        int hitCounts=0;
        int* hits=__str_kmp_search(origin,originlen,old_jc,oldlen,&hitCounts);
        if(hits==NULL){
                free(hits);
                hits=NULL;
                return origin;
        }

        JStr tmp=jstr_newempty();
        int lastEnd=0;

        for(int i=0;i<hitCounts;i++){
                JStr prev=__ansi_subs(origin,originlen,lastEnd,hits[i]);
                
                logc("prev<%s>",prev);
                tmp=__str_cat(tmp,jptr(tmp)->len,prev,jptr(prev)->len);
                tmp=__str_cat(tmp,jptr(tmp)->len,new_jc,newlen);
                jstr_free(prev);
                prev=NULL;
                lastEnd=hits[i]+oldlen;
        }
        
        if(lastEnd<originlen){
                JStr last_snis=__ansi_subs(origin,originlen,lastEnd,-1);
                tmp=__str_cat(tmp,jptr(tmp)->len,last_snis,jptr(last_snis)->len);
                jstr_free(last_snis);
                last_snis=NULL;
        }

        if(tmp!=NULL){
                jstr_free(origin);
                origin=NULL;
                origin=jstr_newlen(tmp,jptr(tmp)->len);
                jstr_free(tmp);
                tmp=NULL;
        }

        free(hits);
        return origin;
}


static JStr __str_replace(JStr origin,int originlen,const char** old_jcs,int old_jcs_len,const char* new_jc){
        int newlen=strlen(new_jc);
        JStr copy_origin=jstr_newlen(origin,originlen);
        for(int i=0;i<old_jcs_len;i++){
                //kmp算法匹配字符，并返回字符匹配数组和数组长度
                int hitCounts=0;
                int* hits=__str_kmp_search(copy_origin,jptr(copy_origin)->len,old_jcs[i],strlen(old_jcs[i]),&hitCounts);
                if(hits==NULL) {
                        free(hits);
                        hits=NULL;
                        goto cleanup;
                }
                int oldlen=strlen(old_jcs[i]);
                
                //循环替换匹配
                //每次只在字符串中替换一个匹配字符
                //匹配完后更新字符串内容，并返回初始循环中用kmp匹配，而后进入内部循环进行替换处理
                //如此反复，直至匹配完成
                JStr tmps=jstr_newempty();
                int lastEnd=0;
                //因为kmp匹配数组的排序是有序的，即匹配下标从小到大排列(从字符段头部至尾部)
                for(int i=0;i<hitCounts;i++){
                        //命中字符前段
                        JStr prev=__ansi_subs(copy_origin,jptr(copy_origin)->len,lastEnd,hits[i]);
                        //逐个字符串拼接(前段+新字符)
                        tmps=__str_cat(tmps,jptr(tmps)->len,prev,strlen(prev));
                        tmps=__str_cat(tmps,jptr(tmps)->len,new_jc,newlen);

                        jstr_free(prev);
                        prev=NULL;
                        lastEnd=hits[i]+oldlen;
                }
                
                if(lastEnd<jptr(copy_origin)->len){
                        JStr last_sni=__ansi_subs(copy_origin,jptr(copy_origin)->len,lastEnd,-1);
                        tmps=__str_cat(tmps,jptr(tmps)->len,last_sni,strlen(last_sni));
                        jstr_free(last_sni);
                        last_sni=NULL;
                }
                
                copy_origin=jstr_copy(tmps);
                jstr_free(tmps);
                tmps=NULL;
                hitCounts=0;
                free(hits);
                hits=NULL;
        }

        if(copy_origin!=NULL){
                jstr_free(origin);
                origin=NULL;
                origin=jstr_copy(copy_origin);
                jstr_free(copy_origin);
                copy_origin=NULL;
        }
        return origin;

cleanup:
        loge("error...hitCoutns or hits[] is NULL,so return origin.");
        return origin;
}


//会改变原有字符！！
static JStr __str_cat(JStr old_jc,int old_len,const void* t,int len){
        //一定要检测参数是否属于JString类型,否则会出现不可预知错误！
        old_jc=__str_onBaseMakeMem(old_jc,len);
        if(old_jc==NULL) return old_jc;

        vjptr(old_jc,jstr);
        memcpy(old_jc+old_len,t,len);
        __str_addEndMark(old_jc,old_len+len);

        jstr->len=len+old_len;
        jstr->free=jstr->free-len;
        *jstr->isJstr='1';
        
        return old_jc;
}


//会改变原有字符！！
static JStr __str_reset(JStr jc, const char *t){
        int len=strlen(t);
        jc=__str_overwriteMakeMem(jc,len);
        if(jc==NULL) return jc;

        memcpy(jc,t,len);
        __str_addEndMark(jc,len);
        return jc;
}



//会破坏原有字符!
//不是字节索引，是一个字符索引
static JStr __str_insert(JStr jc,const char* insertJc,int startIndex){
        int insertlen=strlen(insertJc);

        //插入点上的字符前移
        //插入点上的字符不算入前后片段中,即保持原始字符原貌
        JStr prev_snippet=__utf_subs(jc,jptr(jc)->len,0,startIndex);
        int endIndex=startIndex;
        JStr after_snippet=__utf_subs(jc,jptr(jc)->len,endIndex,-1);
        
        int prevlen=jptr(prev_snippet)->len;
        int afterlen=jptr(after_snippet)->len;

        int total=prevlen+afterlen+insertlen;
        jc=__str_overwriteMakeMem(jc,total);
        if(jc==NULL) return NULL;

        if(prevlen>0){
                memcpy(jc,prev_snippet,prevlen);
        }

        memcpy(jc+prevlen,insertJc,insertlen);
        memcpy(jc+prevlen+insertlen,after_snippet,afterlen);
        __str_addEndMark(jc,total);

        jstr_free(prev_snippet);
        jstr_free(after_snippet);
        return jc;
}

static JStr* __str_splitarrs(const JStr texts,int textslen,const char** signarrs,int signarrlens,int* resultlen){
        int inits=10;
        JStr* jarrs=NULL;
        jarrs=malloc(sizeof(JStr)*inits);
        if(jarrs==NULL) goto cleanup;

        //将后面的标志全都替换为相同的一个标志，方便后续文本分割处理
        JStr copy_texts=__str_replace(texts, textslen,signarrs,signarrlens,signarrs[0]);

        int hitCounts=0;
        int* hits=__str_kmp_search(copy_texts,jptr(copy_texts)->len,signarrs[0],strlen(signarrs[0]),&hitCounts);
        if(hits==NULL) goto cleanup;

        int lastEnd=0;
        int matcheds=0;
        for(int i=0;i<hitCounts;i++){
                if(matcheds+2>inits){
                        inits *=2;
                        JStr* tmps=realloc(jarrs,_IN(inits));
                        if(tmps==NULL) goto cleanup;
                        jarrs=tmps;
                        logc("reallocated memory successfully.");
                }

                JStr prev=__ansi_subs(copy_texts,jptr(copy_texts)->len,lastEnd,hits[i]);
                jarrs[i]=prev;
                lastEnd=hits[i]+strlen(signarrs[0]);
                matcheds+=1;
        }
        if(lastEnd<textslen){
                JStr last_sni=__ansi_subs(texts,textslen,lastEnd,-1);
                jarrs[matcheds]=last_sni;
        }

       if(resultlen==NULL) goto cleanup;
       *resultlen=matcheds;
       return jarrs;
cleanup:
       if(jarrs) free(jarrs);
       if(hits) free(hits);
       return NULL;
}

static JStr* __str_splits(const JStr texts,int textslen,const char* signals,int* resultlen){
        int inits=10;
        JStr* jarrs=NULL;
        jarrs=malloc(sizeof(JStr)*inits);
        if(jarrs==NULL) return NULL;

        int siglen=strlen(signals);
        int hitCounts=0;
        int* hits=__str_kmp_search(texts,textslen,signals,siglen,&hitCounts);
        if(hits==NULL) goto cleanup;

        int lastEnd=0;
        for(int i=0;i<hitCounts;i++){
                if(i+2>inits){
                        JStr* tmp=NULL;
                        inits *=2;
                        tmp=realloc(jarrs,sizeof(JStr)*inits);
                        if(tmp==NULL) goto cleanup;
                        jarrs=tmp;
                }

                JStr prev=__ansi_subs(texts,textslen,lastEnd,hits[i]);
                jarrs[i]=prev;
                lastEnd=hits[i]+siglen; 
        }

        if(lastEnd<textslen){
                JStr last_sni=__ansi_subs(texts,textslen,lastEnd,-1);
                jarrs[hitCounts]=last_sni;
        }

        if(resultlen==NULL) goto cleanup;
        *resultlen=hitCounts;
        return jarrs;
cleanup:
        if(jarrs) free(jarrs);
        if(hits) free(hits);
        return NULL;
}


JStr __str_tostr(int num){
        int digits=__str_getdigits(num); 
        JStr jc=jstr_newlen("",digits);
        if(jc==NULL) return "0";
        int i=0;
        if(num<0){
                num=-num;
                jc[i]='-';
                i++;
        }

        do{
                //取num最低位
                //字符0-9的ASCII码是48-57
                //所以需要加上48;
                jc[i++]=num%10+48;
                num/=10;//去掉最低位
        }while(num);

        jc[i]='\0';
        int j=0;
        if(jc[0]=='-'){
                j=1;
                ++i;
        }

        //对称交换
        for(;j<i/2;j++){
                char tmp=jc[j];
                jc[j]=jc[i-j-1];
                jc[i-j-1]=tmp;
        }

        return jc;
}

int __str_tointlen(const void* jc,int size){
        JStr chars=jstr_newlen(jc,size);

        if(chars==NULL) return 0;
        if(chars[0]=='\0') return 0;
        
        while(*chars==' '){
                chars++;
        }

        int flag=1;
        if(*chars=='-'||*chars=='+'){
                flag=*chars=='-'?-1:1;
                chars++;
        }

#define INT_MAX 2147483648
#define INT_MIN -2147483648
        long long ret=0;
        while(*chars>='0'&&*chars<='9'){
                //从左到右的转换数字
                //*10的作用是使ret整个数字的位数向左移一位，如50,50*10=>500,位数由2变为3
                ret=ret*10+(*chars-'0')*flag;
                if(ret<INT_MIN||ret>INT_MAX) return 0;
                chars++;
        }
        
        char last=*chars;
        
        //释放申请到的内存空间;
        int count=size+1;
        while(--count>0)
                chars--;
        jstr_free(chars);

        if(last=='\0') {
                return (int)ret;
        }

        return (int)ret;
}


/*----public methods-----*/
JStr jstr_newlen(const void* str,int len){
        JString* jstr=NULL;
       
        //不要使用memset函数，会造成性能下降
        jstr=malloc(sizeof(JString)+_CH(len+1));
        if(jstr==NULL) return NULL;

        jstr->len=len;
        jstr->free=0;
        //初始化标识符
        *jstr->isJstr='1';

        if(len&&str)
                memcpy(jstr->buf,str,len);
        __str_addEndMark(jstr->buf,len);
        return (char*) jstr->buf;
}


JStr jstr_newempty(){
         return jstr_newlen("",0);
}


JStr jstr_new(const char* inits){
       size_t initlen=(inits==NULL)?0:strlen(inits);
       return jstr_newlen(inits,initlen);
}

JStr jstr_autofit(JStr jc){
        if(jc==NULL) return NULL;
        assertJc(jc,NULL);
        JStr tmp=realloc(jc,sizeof(JStr)+_CH(jptr(jc)->len+1));
        if(tmp==NULL) return NULL;
        jptr(jc)->free=0;
        jc=tmp;
        return jc;
}

void jstr_expandLen(JStr jc,int newlen){
        if(jc==NULL) return;
        assertJc(jc,);

        if(jptr(jc)->len>newlen){
                int oldlen=jptr(jc)->len;
                jptr(jc)->len=newlen;
                jptr(jc)->free=oldlen-newlen;
                return;
        }

        jc=__str_overwriteMakeMem(jc,newlen);
}

int jstr_len(const JStr jc){
        if(jc==NULL){
                logc("jc<%s> is null",jc);
                return 0;
        }
        assertJc(jc,0)
        return jptr(jc)->len;
}

int jstr_tointlen(const char* jc,int size){
        if(jc==NULL) return 0;
        return __str_tointlen(jc,size);
}
int jstr_toint(const JStr jc){
        if(jc==NULL) return 0;
        assertJc(jc,0);
        return __str_tointlen(jc,jptr(jc)->len);
}

JStr jstr_tostr(int nums){
        return __str_tostr(nums);
}

int jstr_isnum(const JStr jc){
        if(jc==NULL) return 0;
        assertJc(jc,0);

        int check=0;
        int len=jptr(jc)->len;
        if(len==1&&(jc[0]>='0'&&jc[0]<='9')) check=1; 
        while(len--){
                if(jc[len]>='0'&&jc[len]<='9') check=1;
        }
        return check;
}

int jstr_isnumlen(const char* jc,int len){
        if(jc==NULL) return 0;
        int check=0;
        if(len==1&&(jc[0]>='0'&&jc[0]<='9')) check=1; 
        while(len--){
                if(jc[len]>='0'&&jc[len]<='9') check=1;
        }
        return check;
}

JStr jstr_copy(const JStr jc){
        if(jc==NULL) {
                loge("jc<%s> is null",jc);
                return NULL;
        }
        assertJc(jc,NULL);
        return jstr_newlen(jc,jptr(jc)->len);
}

Boolean jstr_isNone(const JStr jc){
        if(jc==NULL){
                loge("jc<%s> is null",jc);
                return TRUE;
        }
        //在jc很大，strlen函数耗费时间会比较久
        assertJc(jc,TRUE)
        return jptr(jc)->len>0?FALSE:TRUE;
}

//失败返回0;-1是末尾,0 是开头;负数是逆序,正数是正序
char jstr_charAt(const JStr jc,int index){
        if(jc==NULL){
                loge("jc<%s> is null",jc);
                return '0';
        }

        //这里假定传入字符一定不是malloc分配的内存，而是系统或compiler分配的
        //如果 strlen()的数值非常大，说明这是非法的.
        //同时也要检测传入字符是否属于JSTR类型
        assertJc(jc,'0')
        int jclen=jptr(jc)->len;
        
        if(index>jclen) return '0';
        //检测是否包含ANSI之外的字符，如果有则截取失败！
        List* utfs=__utf_getUtfobjs(jc,jclen);
        int isUtf=1;
        list_for(utfs,cur){
                struct __Utfobj* obj=(struct __Utfobj*)cur->obj;
                if(obj==NULL){
                        list_release(utfs);
                        return 0;
                }
                if(obj->len>1){
                      isUtf=0;
                      break;
                }
        }

        if(isUtf==0){ 
                Err("failed to get a character because the string includes utf-encoded characters.",jc,s);
                return '0';
        }

        if(index<0) index=jclen+index;
        list_release(utfs);
        return jc[index];
}


//正数顺序截取,负数逆序截取
JStr jstr_subAll(const JStr jc, int startIndex){
        if(jc==NULL) {
                loge("jc<%s> is null",jc);
                return NULL;
        }
        
        assertJc(jc,NULL)
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        int strlens=jptr(jc)->len;

        int start=__str_abs(startIndex);
        if(start>=strlens){
                loge("start<%d> is larger than strlens<%d>",start,strlens);
                return NULL;
        }

        return __utf_subs(jc,strlens,startIndex,-1);
}

JStr jstr_subs(const JStr jc, int startIndex, int endIndex){
        if(jc==NULL){
                loge("jc<%s> is null",jc);
                return NULL;
        }
        assertJc(jc,NULL)
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错     
        int strlens=jptr(jc)->len;
        
        return __utf_subs(jc,strlens,startIndex,endIndex);
}

JStr jstr_subsAnsi(const JStr jc,int startIndex,int endIndex){
        if(jc==NULL) {
                loge("jc<%s> is null",jc);
                return NULL;
        }
        
        assertJc(jc,NULL)
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        int strlens=jptr(jc)->len;

        int start=__str_abs(startIndex);
        int end=__str_abs(endIndex);
        
        if(start>strlens||end>strlens){
                loge("start<%d>(startIndex<%d>) or end<%d>(endIndex<%d>) is too large than strlens<%d>",start,startIndex,end,endIndex,strlens);
                return NULL;
        } 
        return __ansi_subs(jc,strlens,startIndex,endIndex);
}



JStr jstr_reverse(const JStr jc){
        if(jc==NULL) {
                loge("jc<%s> is null",jc);
                return NULL;
        }
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        assertJc(jc,NULL)
        return __utf_reverse(jc,jptr(jc)->len,0,-1);
}

JStr jstr_subverse(const JStr jc, int start, int end){
        if(jc==NULL) {
                loge("jc<%s> is null",jc);
                return NULL;
        }
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        assertJc(jc,NULL)
        return __utf_reverse(jc,jptr(jc)->len,start,end);
}

Boolean jstr_equal(const JStr jc1,const JStr jc2){
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        if(jc1==NULL||jc2==NULL) {
                loge("jc1<%s> or jc2<%s> is null",jc1,jc2);
                return FALSE;
        }
        assertJc(jc1,0);
        assertJc(jc2,0);
        if(jptr(jc1)->len!=jptr(jc2)->len) return FALSE;

        int jclen=jptr(jc1)->len;
        return __str_equal(jc1,jclen,jc2,jclen);
}


Boolean jstr_startsWith(const JStr jc,const char* signals){
        if(jc==NULL||signals==NULL) {
                loge("jc<%s> or signals<%s>is null.",jc,signals);
                return FALSE;
        }

        assertJc(jc,FALSE);
        if(jptr(jc)->len<strlen(signals)) return FALSE;
        return __str_starts(jc,signals,0);
}

//匹配成功 1，失败0
Boolean jstr_endsWith(const JStr jc,const char* signals){
        if(jc==NULL||signals==NULL) {
                loge("jc<%s> or signals<%s>is null.",jc,signals);
                return FALSE;
        }
        assertJc(jc,FALSE)
        if(jptr(jc)->len<strlen(signals)) return FALSE;
        return __str_ends(jc,jptr(jc)->len,signals,0);
}

//indexof类型 失败一律返回-1
//指定字符第一次出现的索引位置
int jstr_indexOf(const JStr origin,const char* signals){
        if(origin==NULL||signals==NULL) {
                loge("jc<%s> or signals<%s>is null.",origin,signals);
                return -1;
        }
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        assertJc(origin,-1)
        if(jptr(origin)->len<strlen(signals)) return -1;
        
        int siglen=strlen(signals);
        int hitCounts=0;
        int* hits=__str_kmp_search(origin,jptr(origin)->len,signals,siglen,&hitCounts);
        if(hits==NULL) return -1;

        int index=hits[0];
        free(hits);
        return index;
}

int jstr_lastIndexOf(const JStr origin,const char* signals){
        if(origin==NULL||signals==NULL){
                loge("jc<%s> or signals<%s>is null.",origin,signals);
                return -1;
        }

        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        assertJc(origin,-1)
        if(jptr(origin)->len<strlen(signals)) return -1;
        
        int siglen=strlen(signals);
        int hitCounts=0;
        int* hits=__str_kmp_search(origin,jptr(origin)->len,signals,siglen,&hitCounts);
        if(hits==NULL) return -1;
        
        int index=hits[hitCounts];
        free(hits);
        return index;
}

int jstr_contains(const JStr origin,const char* signals){
        if(origin ==NULL||signals==NULL) return 0;
        assertJc(origin,0);
        int hitCounts=0;
        int* hits=__str_kmp_search(origin,jptr(origin)->len,signals,strlen(signals),&hitCounts);
        if(hits==NULL) {
                if(hits) free(hits);
                return 0;
        }

        if(hits) free(hits);
        return hitCounts;
}

/**----实现难度较大的函数,且会改变字符内容-----**/
JStr jstr_reset(JStr old_jc, const char* new_jc){
        if(old_jc==NULL||new_jc==NULL){
                loge("old_jc<%s> or new_jc<%s> maybe null.",old_jc,new_jc);
                return NULL;
        }
        assertJc(old_jc,NULL)
        old_jc=__str_reset(old_jc,new_jc);
        return old_jc;
}

JStr jstr_displaces(JStr origin, const char* old_jc,const char* new_jc){
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        if(old_jc==NULL||new_jc==NULL) {
                loge("old_jc<%s> or new_jc<%s>",old_jc,new_jc);
                return NULL;
        }
        assertJc(origin,NULL); 
        if(strlen(old_jc)>jptr(origin)->len) return NULL;
        origin=__str_displace(origin,jptr(origin)->len,old_jc,new_jc);
        return origin;
}

//建议调用宏函数jstr_replace(texts,new_jc,...)
JStr jstr_replaces(JStr origin, const char** old_jcs,int old_jcs_len,const char* new_jc){
        //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错
        if(old_jcs==NULL||new_jc==NULL) {
                loge("old_jc<%s> or old_jcs,new_jc<%s>",old_jcs[0],new_jc);
                return NULL;
        }
        assertJc(origin,NULL); 
        origin=__str_replace(origin,jptr(origin)->len,old_jcs,old_jcs_len,new_jc);
        return origin;
}

JStr jstr_insert(JStr jc,const char* insertJc,int startIndex){
         //不要随意使用jptr寻找结构体,因为jc不一定属于JString类型！！
        //应该先用assertJc进行判断,防止出错 
        if(jc==NULL||insertJc==NULL) {
                loge("jc<%s> or insertJc<%s> is null.",jc,insertJc);
                return NULL;
        }
        assertJc(jc,NULL)
        jc=__str_insert(jc,insertJc,startIndex);
        return jc;
}

JStr jstr_cat(JStr jc1,const char* jc2){
        if(jc1==NULL||jc2==NULL) {
                loge("jc1<%s> or jc2<%s> is null",jc1,jc2);
                return NULL;
        }
        assertJc(jc1,NULL);
        jc1=__str_cat(jc1,jptr(jc1)->len,jc2,strlen(jc2));
        return jc1;
}

JStr* jstr_splits(const JStr texts,const char* signals,int* resultlen){
        if(signals==NULL||texts==NULL) {
                loge("texts<%s> or signals<%s>", texts, signals);
                return NULL;
        }
        assertJc(texts,NULL)
        if(resultlen==NULL){
                //我们需要确保resultlen指针已经初始化，否则就给指定一个地址
                Err("is not initialized,you must sopecify the adress on it!",resultlen,p);
                return NULL;
        }
        JStr* tmp=__str_splits(texts,jptr(texts)->len,signals,resultlen);
        return tmp;
}

//建议调用宏jstr_slit(const texts,int* resultlen,...)
JStr* jstr_slits(const JStr texts,const char** signal_arr,int signal_arr_len,int* resultlen){
        if(texts==NULL||signal_arr==NULL) return NULL;
        assertJc(texts,NULL);
        if(resultlen==NULL){
                //我们需要确保resultlen指针已经初始化，否则就给指定一个地址
                Err("is not initialized,you must sopecify the adress on it!",resultlen,p);
                return NULL;
        }
        JStr* tmp=__str_splitarrs(texts,jptr(texts)->len,signal_arr,signal_arr_len,resultlen);
        return tmp;
}

void jstr_free(JStr jc){
        if(jc==NULL) return;
        assertJc(jc,)
        
        JString *jstr=jptr(jc);
        free(jstr);
        jstr=NULL;
}

void jstr_frees(JStr* jrrs,int len){
        if(!jrrs) return;
        while(len--)
                jstr_free(jrrs[len]);
        free(jrrs);
}

//测试范例
void jstr_test(){
        //--jstr_split&&jstr_replace--/
        FILE* f=fopen("/home/hazukie/cprojects/exercise_c/long.txt","r");
        char line[1024];
        JStr longexts=jstr_newempty();
        
        int record=0;
        while(fgets(line,1024,f)){
                longexts=jstr_cat(longexts,line);
        }
        fclose(f);

        longexts=jstr_replace(longexts,"<br>","\n");
        logi(longexts,s,"");
        //---jstr_replace--/

        //---jstr_splits--/
        char* examples=jstr_new("name:hello,world\nage:20;class:2041;tel:19914834737|email:263@hazu.com");
       
        int examplen=0;
        JStr* exampleJarrs=jstr_slit(examples,&examplen,";","\n","\t","|");
        for(int i=0;i<examplen;i++)
                printf("=>[%s]\n",exampleJarrs[i]);
        puts("\n");

        int longlens=0;
        JStr* jss=jstr_splits(longexts,"<br>",&longlens);
       
        jstr_frees(exampleJarrs,examplen);

        for(int i=0;i<longlens;i++)
               printf("==> \033[0;36m%s\n\033[0m",jss[i]);
        jstr_frees(jss,longlens);
        jstr_free(longexts);
        //---jstr_splits--/


        //---jstr_insert---/
        char* j1=jstr_new("吾系客家人");
        char* j2=jstr_insert(j1,",好食客家菜！",-1);
        logi(j2,s,"");
        //---jstr_insert---/


        //---jstr_displace--/
        char* distr=jstr_new("吾系客家人，好食客家菜！客家人系真勤劳个人呐！Installation packages for Audacity are provided by many GNU/Linux and Unix-like distributions. Use the distribution’s usual package manager (where available) to install Audacity. If necessary, you could try searching for an appropriate Audacity package on rpmseek.");
        
        logi(jstr_contains(distr,"Audacity"),d,"");

        char* c=jstr_replace(distr,"**","Audacity");
        logi(c,s,"");
        //---jstr_displace--/
        
        //---jstr_toint---/
        JStr jci=jstr_new("12345678");
        int isa=jstr_toint(jci);
        logi(isa,d,"");

        int iza=jstr_tointlen("12345678",sizeof(char)*8);
        logi(iza,d,"");

        int sin=987654321;
        if(jstr_isnum(jci)==1) logc("jci<%s> is a number.",jci);
        logi(jstr_tostr(sin),s,"");
        //---jstr_toint---/
}


