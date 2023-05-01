#include "JRpn.h"
#include "JList.h"
#include "JStack.h"
#include "JString.h"
#include <stdio.h>

char getOp_Priority(char c1,char c2){
        char result;
        switch(c2){
                case '+':
                case '-': 
                        if(c1=='('||c1=='#') result='<';
                        else result = '>'; 
                        break;
                case '*':
                case '/':
                        if(c1=='*'||c1=='/'||c1==')') result='>';
                        else result = '<';
                        break;
                case '%':
                        if(c1=='*'||c1=='/'||c1=='%'||c1==')') result='>';
                        else result = '<';
                        break;
                case '(':
                        if(c1==')'){
                                printf("error<%c>\n",c1);
                                goto exit;
                        }
                        else result = '<';
                        break;
                case ')':
                        switch(c1){
                                case '(':
                                        result='=';
                                        break;
                                case '#':
                                        printf("c1<%c> lack of left quote<%c>\n",c1,c2);
                                        goto exit;
                                default:
                                        result='>';
                        }
                        break;
                case '#':
                        switch(c1){
                                case '#':
                                        result='=';
                                        break;
                                case '(':
                                        printf("c1<%c> lack of right quote<%c>\n",c1,c2);
                                        goto exit;
                                default:
                                        result='>';
                        }
        }
        
        return result;
exit:
        return '0';
}


int isOp(char c1){
        int check=(
                        c1=='+'||
                        c1=='-'||
                        c1=='*'||
                        c1=='/'||
                        c1=='%'||
                        c1=='('||
                        c1==')'||
                        c1=='#')
                ?1:0;

        return check;
}


JStr operate(const char* jc1,char op,const char* jc2){
        int result=0;
        int ijc1=jstr_tointlen(jc1,strlen(jc1));
        int ijc2=jstr_tointlen(jc2,strlen(jc2));
        //printf("jc1:%s,jc2:%s\n",jc1,jc2);
        switch(op){
                case '+':
                        result = ijc1+ijc2;
                        break;
                case '-':
                        result = ijc1-ijc2;
                        break;
                case '*':
                        result = ijc1*ijc2;
                        break;
                case '/':
                        result = ijc1/ijc2;
                case '%':
                        result=ijc1%ijc2;
                        break;
        }
        return jstr_tostr(result);
}

//分割字符串中的数字字符
List* splitExprs(JStr exprs){
        List* l=list_init(LCHAR);
        //printf("exprs->%s,lens:%d\n",exprs,jstr_len(exprs));
        
        int lastEnd=0,count=0,start=0,len=0;
        int lens=jstr_len(exprs);
        JStr subs=NULL;
        while(--lens>0){
                if(
                   exprs[len]=='-'||
                   exprs[len]=='+'||
                   exprs[len]=='*'||
                   exprs[len]=='/'||
                   exprs[len]=='%'||
                   exprs[len]=='('||
                   exprs[len]==')'){
                        start=len;
                        int end=lastEnd+count;
                        //printf("start=%d,end=%d,count=%d\n",lastEnd,end,count);
                        if(lastEnd!=end){
                                subs=jstr_subsAnsi(exprs,lastEnd,end);
                                //printf("subs_str->%s\n",subs);
                                list_add(l,subs,sizeof(char)*count);
                        }
                        list_add(l,&exprs[len],1);
                        //printf("add_op:%c\n",exprs[len]);
                        count=0; 
                        lastEnd=end==0?1:end+1;
                }
                else count++;
                
                len++;
        }       

        //因为lastEnd是包含开头部分的,比如3-4即: 3也包含在内,但4不包含在内
        //而len是从0开始计算的,lastEnd是从1开始计算的
        if(lastEnd<len+1){
                subs=jstr_subsAnsi(exprs,lastEnd,-1);
                //printf("subs_str->%s\n",subs);
                list_add(l,subs,sizeof(char)*(len-lastEnd+1));
                list_add(l,"#",2);
        }
        
        if(subs!=NULL) jstr_free(subs);
        return l;
}

char* calcExprs(List* exprs){
        //数字存储
        JStack* OPND=jstack_init();
        //运算符存储
        JStack* OPTR=jstack_init();

        jstack_push(OPTR,"#",2);

        int eslen=exprs->len;
        //printf("eslen=%d\n",eslen);

        int exprslen=0;
        char* a;
        char* b;
        char* c;
        char* x;
        x=(char*)jstack_top(OPTR);
        c=(char*)list_get(exprs,exprslen++);

        while(exprslen<eslen+1&&(c[0]!='#'||x[0]!='#')){
                if(strlen(c)==1&&isOp(c[0])){
                        printf("OPTR:[ ");
                        jstack_for(OPTR,cur)
                                printf("%s ",(char*)cur->data);
                        printf("]\n");
                        
                        printf("OPND:[ ");
                        jstack_for(OPND,lcur)
                                printf("%s ",(char*)lcur->data);
                        printf("]\n\n");

                        switch(getOp_Priority(x[0],c[0])){
                                case '<':
                                        printf("push op<%s>\n",c);
                                        jstack_push(OPTR,c,2);
                                        c=(char*)list_get(exprs,exprslen++);
                                        break;
                                case '=':
                                        printf("pop <%s> with c<%s>\n",(char*)jstack_top(OPTR),c);
                                        jstack_pop(OPTR);
                                        c=(char*)list_get(exprs,exprslen++);
                                        break;
                                case '>':
                                        printf("pop op<%s> with c<%s>\n",(char*)jstack_top(OPTR),c);
                                        x=(char*)jstack_pop(OPTR);
                                        b=(char*)jstack_pop(OPND);
                                        a=(char*)jstack_pop(OPND);

                                        if(a!=NULL&&a!=NULL&&x!=NULL){
                                                JStr operated_result=operate(a,x[0],b); 
                                                printf("%s%s%s=%s\n",a,x,b,operated_result);
                                                jstack_push(OPND,operated_result,jstr_len(operated_result)+1);
                                                printf("push operated_num<%s>\n",operated_result);
                                        }else{
                                                printf("errors.(happen when a<%s>,x<%c>,b<%s>\n",a,x[0],b);
                                                return "errors";
                                        }
                                        break;
                                case '0':
                                        /*printf("found error,try to pop one op<%s> from OPTR...\n",(char*)jstack_top(OPTR));
                                        jstack_pop(OPTR);
                                        break;*/
                                        printf("error<%s>\n",(char*)jstack_top(OPTR));
                                        return "errors";
                        }                                
                }
                else if(jstr_isnumlen(c,strlen(c))){
                        jstack_push(OPND,c,sizeof(c));
                        printf("push_num:%s\n",c);
                        c=(char*)list_get(exprs,exprslen++);
                        //printf("next_num:%s\n",c);
                }else{
                        printf("Illegal exprs<%s>\n",c);
                        return "error";
                }
                

                x=(char*)jstack_top(OPTR);
                //printf("exprslen:%d\n\n",exprslen);
        }
       
        x=(char*)jstack_pop(OPND);
        if(jstack_isnone(OPND)==0){
                printf("exprs have errors<%s>.\n",x);
                return "error";
        }
        return x;
}

int main(void){

        char* s="199*(20+2)*2+10010+10000*125";
        JStr expression=jstr_new(s);
        List* l=splitExprs(expression);
        char* res=calcExprs(l);
        printf("final_result of %s: %s\n\n",s,res);

        list_for(l,cur){
                char* chars=(char*)cur->obj;
                printf("chars:%s\n",chars);
        }  
        return 0;
}

