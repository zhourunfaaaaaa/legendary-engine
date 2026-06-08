#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} List;

void free_list(List *list)
{
    Node *p = list->head;
    while (p != NULL) {
        Node *q = p->next;
        free(p);
        p = q;
    }
    list->head = NULL;
    list->tail = NULL;
}

void search(List *list)
{
    int number;
    printf("输入要查询的数字：");
    scanf("%d", &number);
    while (number != -1) {
        if (list->head == NULL) {
            printf("链表为空！\n");
        } else {
            Node *p = list->head;
            while (p != NULL) {
                if (p->value == number) {
                    printf("找到了！\n");
                    break;
                }
                p = p->next;
                if (p == NULL) {
                    printf("未找到！\n");
                }
            }
        }
        printf("输入要查询的数字：");
        scanf("%d", &number);
    }
}

int main()
{
    List list;
    list.head = NULL;
    list.tail = NULL;
    int number;
    scanf("%d", &number);
    while (number != -1) {
        Node *node = (Node*)malloc(sizeof(Node));
        node->value = number;
        node->next = NULL;
        if (list.head == NULL) {
            list.head = node;
            list.tail = node;
        } else {
            list.tail->next = node;
            list.tail = node;
        }
        scanf("%d", &number);
    }

    Node *p = list.head;
    while (p != NULL) {
        printf("%d\n", p->value);
        p = p->next;
    }

    search(&list);

    free_list(&list);

    return 0;
}
