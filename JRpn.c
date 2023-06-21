#include "JRpn.h"

static char getOp_Priority(char c1,char c2){
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
                case '^':
                        if(c1=='^'||c1==')') result='>';
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


static int isOp(char c1){
        int check=(
                        c1=='+'||
                        c1=='-'||
                        c1=='*'||
                        c1=='/'||
                        c1=='%'||
                        c1=='^'||
                        c1=='('||
                        c1==')'||
                        c1=='#')
                ?1:0;

        return check;
}

static int int_sqrt(int m,int n){
        /*int result=1;
        while(n!=1){
                if(n&1==1) result*=m;
                m*=m;
                n>>=1;
        }
        return result;*/
        int i=1;
        int result=1;
        if(n==0) return result;
        for(;i<=n;i++) result*=m;
        return result;
}

static JStr operate(const char* jc1,char op,const char* jc2){
        int result=0;
        int ijc1=jstr_tointlen(jc1,strlen(jc1));
        int ijc2=jstr_tointlen(jc2,strlen(jc2));
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
                        break;
                case '%':
                        result=ijc1%ijc2;
                        break;
                case '^':
                        result=int_sqrt(ijc1,ijc2);
                        break;
        }
        return jstr_tostr(result);
}


static double flt_sqrt(double m,double n){
        /*int result=1;
        while(n!=1){
                if(n&1==1) result*=m;
                m*=m;
                n>>=1;
        }
        return result;*/
        int i=1;
        double result=1;
        if(n==0) return result;
        for(;i<=n;i++) result*=m;
        return result;
}

static JStr operateflt(const char* jc1,char op,const char* jc2){
        double result=0.0;
        double ijc1=atof(jc1);
        double ijc2=atof(jc2);
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
                        break;
                case '%':
                        result=(int)ijc1%(int)ijc2;
                        break;
                case '^':
                        result=flt_sqrt(ijc1,ijc2);
                        break;
        }
        JStr neret=jstr_newlen("",11);
        sprintf(neret,"%f",result);
        return neret;
}


//分割字符串中的数字字符
static List* splitExprs(const JStr exprs){
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
                   exprs[len]=='^'||
                   exprs[len]=='('||
                   exprs[len]==')'){
                        start=len;
                        int end=lastEnd+count;
                        //printf("start=%d,end=%d,count=%d\n",lastEnd,end,count);
                        if(lastEnd!=end){
                                subs=jstr_subsAnsi(exprs,lastEnd,end);
                                //printf("subs_str->%s\n",subs);
                                list_add(l,subs,sizeof(char)*(jstr_len(subs)+1));
                        }
                        char n[2];
                        *n=exprs[len];
                        *(n+1)='\0';
                        list_add(l,n,2);


                        jstr_free(subs);
                        subs=NULL;
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
                list_add(l,subs,sizeof(char)*(jstr_len(subs)+1));
                list_add(l,"#",2);
        }
        
        if(subs!=NULL) jstr_free(subs);
        return l;
}

static char* calcExprs(const JStr jcs,int type){
        if(jcs==NULL) return NULL;
        List* exprs=splitExprs(jcs);
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
                        printf("\n\n<------START------>\n");
                        printf("OPTR=>[ ");
                        jstack_for(OPTR,cur)
                                printf("%s ",(char*)cur->data);
                        printf("]\n");
                        
                        printf("OPND=>[ ");
                        jstack_for(OPND,lcur)
                                printf("%s ",(char*)lcur->data);
                        printf("]\n");

                        switch(getOp_Priority(x[0],c[0])){
                                case '<':
                                        printf("\033[0;31mpush_op<%s>\033[0m\n",c);
                                        jstack_push(OPTR,c,2);
                                        c=(char*)list_get(exprs,exprslen++);
                                        break;
                                case '=':
                                        printf("\033[0;32mpop_op<%s> with c<%s>\033[0m\n",(char*)jstack_top(OPTR),c);
                                        jstack_pop(OPTR);
                                        c=(char*)list_get(exprs,exprslen++);
                                        break;
                                case '>':
                                        printf("operated_op<%s> has poped\n",(char*)jstack_top(OPTR));
                                        x=(char*)jstack_pop(OPTR);
                                        b=(char*)jstack_pop(OPND);
                                        a=(char*)jstack_pop(OPND);

                                        if(a!=NULL&&b!=NULL&&x!=NULL){
                                                JStr operated_result=type==0?operate(a,x[0],b):operateflt(a,x[0],b); 
                                                printf("operated_exprs: %s%s%s=%s\n",a,x,b,operated_result);
                                                jstack_push(OPND,operated_result,jstr_len(operated_result)+1);
                                                printf("operated_num<%s> has pushed\n",operated_result);
                                        }else{
                                                printf("errors.(happen when a<%s>,x<%c>,b<%s>\n",a,x[0],b);
                                                goto error;
                                        }
                                        break;
                                case '0':
                                        /*printf("found error,try to pop one op<%s> from OPTR...\n",(char*)jstack_top(OPTR));
                                        jstack_pop(OPTR);
                                        break;*/
                                        printf("error<%s>\n",(char*)jstack_top(OPTR));
                                        goto error;
                        }                                
                }
                else if(jstr_isnumlen(c,strlen(c))){
                        jstack_push(OPND,c,sizeof(c));
                        printf("\033[0;35mpush_num:%s\033[0m\n",c);
                        c=(char*)list_get(exprs,exprslen++);
                        //printf("next_num:%s\n",c);
                }else{
                        printf("Illegal exprs<%s>\n",c);
                        goto error;
                }

                x=(char*)jstack_top(OPTR);
                //printf("exprslen:%d\n\n",exprslen);
        }
       
        x=(char*)jstack_pop(OPND);
        if(jstack_isnone(OPND)==0){
                printf("exprs have errors<%s>.\n",x);
                x="error";
        }

        list_release(exprs);
        jstack_release(OPTR);
        jstack_release(OPND);
        return x;
error:
        list_release(exprs);
        jstack_release(OPND);
        jstack_release(OPTR);
        return "errors.";
}


JStr jrpn_calcInt(const JStr jc){
        if(jc==NULL) return NULL;
        return calcExprs(jc,0);
}

JStr jrpn_calcFlt(const JStr jc){
        if(jc==NULL) return NULL;
        return calcExprs(jc,1);
}


void jrpn_test(){
        char* s="((23/7*(7-3+15)))*(67-22)*71";
        JStr expression=jstr_new(s);
        
        char* res_int=jrpn_calcInt(jstr_new("(1/3)^3+1/2*15+(200-103)%13"));
        char* res_flt=jrpn_calcFlt(expression);
        printf("\n\nfinal_result of %s: %s\n",s,res_flt);
        printf("\nfinal_result of (1/3)^3+1/2*15+(200-103)mod 13: %s\n\n",res_int);

        List* l=splitExprs(expression);
        printf("切割表达式<%s>：\n",expression);
        list_for(l,cur){
                char* chars=cur->obj;
                printf("chars:%s\n",chars);
        }
        
        list_release(l);
        jstr_free(expression);
}

/*int main(void){
        jrpn_test();
        return 0;
}*/
