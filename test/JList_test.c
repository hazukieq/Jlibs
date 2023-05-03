#include <stdio.h>
#include "../JList.h"


struct Lk{
        char* name;
        char numbers[12];
        char* gender;
        char   telenum[12];
        int age;
};



void executeFn(void* obj){
        struct Lk* lq;
        lq=(struct Lk*)obj;
        printf("name: %s,gender: %s,age:%d,numbers: %s,telenum:%s\n",lq->name,lq->gender,lq->age,lq->numbers,lq->telenum);
 
        //printf("cycled: %s\n",((char*)obj));
}

int main(void){
        
        List* l=list_init(LSTRUCT);
        List* nel=list_init(LSTRUCT);

        struct Lk lk={
                .name="hazukie",
                .gender="male",
                .numbers="99999999999",
                .telenum="11111222220",
                .age=19
        };

        struct Lk ilk={
                .name="叶月绘梨依",
                .gender="女性",
                .numbers="34556789012",
                .telenum="134472692270",
                .age=18
        };


        for(int j=0;j<3;j++)
                list_add(l,&lk,sizeof(lk));
        for(int j=0;j<10;j++)
                list_add(nel,&lk,sizeof(lk));

        printf("------ LinkedList test -------\n");
        printf("remove item before len: %d\n",l->len);
        //打印元素
        list_foreach(l,executeFn);


        //添加元素

        Node node={
                .obj=&lk,
                .size=sizeof(lk)
        };

        int check=list_contains(l,&node);
        printf("include node is %d!\n",check);
        
        list_set(l,&ilk,sizeof(ilk),2);
        list_insert(l,&ilk,sizeof(ilk),3);
        list_foreach(l,executeFn);

        list_for(l,current){
            struct Lk* lq;
            if(current!=NULL){
                    lq=(struct Lk*)current->obj;
                    printf("\nGet():\nname: %s,gender: %s,age:%d,numbers: %s,telenum:%s\n",
                                    lq->name,lq->gender,lq->age,lq->numbers,lq->telenum);
            }

        }
        printf("\n");
        printf("add one item into list\n");
        printf("removed item after len: %d\n",l->len);
        printf("\n");
        
        //删除元素
        list_remove(l,1);
        printf("remove one item from list\n");
        printf("list len:%d\n",l->len);

        printf("\n");
       
        list_merge(l,nel);
        list_reverse(l);

        printf("list_merge: len<%d>\n",l->len);
        list_for(l,ncur){
                struct Lk* lq=(struct Lk*)ncur->obj;    
                printf("Lk{ name: %s,gender: %s,age:%d,numbers: %s,telenum:%s }\n",
                                    lq->name,lq->gender,lq->age,lq->numbers,lq->telenum);

        }

        //释放内存
        list_empty(l);

        printf("programme has ran successfully\n");
        printf("------END LinkedList test END-------\n");
        return 0;
}


