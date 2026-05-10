#ifndef _LINKED_LIST
#define _LINKED_LIST_

typedef struct _node {
    int value;
    struct _node* next;
} Node;

typedef struct _list {
    Node* head;
    Node* tail;
} List;

void list_add(List* pList,int number);
void list_print(List* pList);
void list_delete(List* pList,int number);
void list_clear(List* pList);

#endif