#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    Node *next;
} Node;

int main()
{
    Node *head = NULL;
    Node *tail = NULL;
    int number;
    scanf("%d", &number);
    while (number != -1) {
        Node *node = (Node*)malloc(sizeof(Node));
        node->value = number;
        node->next = NULL;
        if (head == NULL) {
            head = node;
            tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
        scanf("%d", &number);
    }
    
    Node *p = head;
    while (p != NULL) {
        printf("%d\n", p->value);
        p = p->next;
    }

    p = head;
    while (p != NULL) {
        Node *q = p->next;
        free(p);
        p = q;
    }
    head = NULL;
    tail = NULL;


    return 0;
}