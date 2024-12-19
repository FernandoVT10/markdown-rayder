#include <stdlib.h>
#include <string.h>
#include "raylib.h"
#include "linklist.h"

LinkList *list_create()
{
    LinkList *list;
    list = malloc(sizeof(*list));

    if(!list) {
        TraceLog(LOG_ERROR, "Couldn't allocate memory for the linked list :)");
        return NULL;
    }

    memset(list, 0, sizeof(*list));
    return list;
}

void list_insert_item_at_end(LinkList *list, void *data)
{
    ListNode *node = malloc(sizeof(ListNode));
    node->next = NULL;
    node->data = data;

    if(list->count == 0) {
        list->head = node;
        list->tail = node;
        list->count++;
        return;
    }

    ListNode *last_node = list->tail;

    last_node->next = node;
    list->tail = node;
    list->count++;
}
