#include "linkedList.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    List list;
    list.head = list.tail = NULL;
    int number;

    do {
        printf("请输入数字（-1停止）：");
        scanf("%d",&number);
        list_add(&list,number);
    } while (number != -1);

    list_print(&list);

    printf("请输入想要删除的数字：");
    int isFound = 0;
    scanf("%d",&number);
    list_delete(&list,number);
    printf("已删除\n现在将清除整个链表\n");
    list_clear(&list);
    return 0;
}

void list_add(List* pList,int number) {
    Node* p = (Node*)malloc(sizeof(Node));
    p->value = number;
    p->next = NULL;
    if (pList->head) {
        // 由于实时记录了tail的位置，所以不需要循环遍历来寻找
        // 接上下一个链表
        pList->tail->next = p;// 让tail此时所指向的链表的next指向新增的p
        pList->tail = p;// 让tail指向p，就是目前链表的结尾
    } else {
        pList->head = p;
        pList->tail = p;
    }
}

void list_print(List* pList) {
    Node* p;
    for (p = pList->head;p;p = p->next) {
        printf("%d\t",p->value);
    }
    printf("\n");
}

void list_delete(List* pList,int number) {
    Node *p,*q;
    for (p = pList->head,q = NULL;p;q = p,p = p->next) {
        if (p->value == number) {
            if (q) {
                q->next = p->next;
            } else {
                pList->head->next = p->next;
            }
            free(p);
            break;
        }
    }
}

void list_clear(List* pList) {
    Node *p,*q;
    p = pList->head;
    while (p) {
        q = p->next;
        free(p);
        p = q;
    }
    pList->head = NULL;
    pList->tail = NULL;
}