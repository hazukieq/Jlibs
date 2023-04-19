#include <stdio.h>
#include "../JString.h"

int main(void){
        JStr js=jstr_new("hello,world!");
        const JStr js1=jstr_new("I am a good boy.");
        
        printf("js<%s>,js1<%s>\n",js,js1);
        
        js=jstr_cat(js,js1);
        printf("js<%s>\n",js);

        js=jstr_replace(js,"*","o");
        printf("%s\n",js);
        int le=0;
        JStr* jarrs=jstr_slit(js,&le,",",".","!"," ");
        for(int i=0;i<le;i++)
                printf("%s\n",jarrs[i]);

        jstr_free(js);
        jstr_free(js1);
        return 0;
}

