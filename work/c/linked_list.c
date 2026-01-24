#include <stdio.h>
#include <stdlib.h>
#include "linked-list.h"

typedef struct list {
    Node *head;
    Node *tail;
}List;// 封装

// 添加链表
list_add(List *list,int number)
{
    Node *p = (Node*)malloc(sizeof(Node));
    p->value = number;
    p->next = NULL;
    if (list->head == NULL) {
        list->head = p;
        list->tail = p;
    } else {
        list->tail->next = p;// 前一个链表指向新的链表
        list->tail = p;// tail指向新的链表
    }
}

// 遍历
void list_print(List *list)
{
    Node *p;
    for(p = list->head ;p ;p = p->next) {
        printf("%d\t",p->value);
    }
    printf("\n");
}

// 删除
void list_delete(List *list,int number)
{
    Node *p = list->head;
    Node *q = NULL;
    Node *t;
    while (p != NULL) {
        t = p->next;
        if (p->value == number) {
            if (q == NULL) {
                list->head = t;
                free(p);
                p = t;
            } else {
                q->next = t;
                free(p);
                p = t;
            }
        } else {
            q = p;
            p = t;
        }
    }
    list->tail = q;
}

// 清除整个链表
void list_clear(List *list)
{
    Node *p;
    Node *q;
    for (p = list->head,q = NULL; p;p = q) {
        q = p->next;
        free(p);
    }
    list->head = NULL;
    list->tail = NULL;
}

int main()
{
    List list;
    list.head = NULL;
    list.tail = NULL;
    int number = 0;
    do {
        scanf("%d",&number);
        if (number != -1) {
            list_add(&list,number);
        }
    } while (number != -1);
    list_print(&list);
    scanf("输入想要删除的数字：%d",&number);
    while (number != -1) {
        list_delete(&list,number);
        scanf("输入想要删除的数字：%d",&number);
    }
    list_clear(&list);
    
    return 0;
}