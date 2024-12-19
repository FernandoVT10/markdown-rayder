#ifndef LINK_LIST_H_
#define LINK_LIST_H_

typedef struct ListNode {
    void *data;
    struct ListNode *next;
} ListNode;

typedef struct LinkList {
    ListNode *head;
    ListNode *tail;
    size_t count;
} LinkList;

LinkList *list_create();
void list_insert_item_at_end(LinkList *list, void *data);

#endif
