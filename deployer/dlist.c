/**
 * @file dlist.c
 * @brief 
 * @author Joe Shang <shangchuanren@gmail.com>
 * @version 
 * @date 2014-04-24
 */

#include "dlist.h"

#include <stdlib.h>

typedef struct _DListNode
{
    struct _DListNode *prev;
    struct _DListNode *next;
    void *data;
}DListNode;

struct _DList
{
    DListNode *head;
    size_t length;
    DataDestroyFunc data_destroy;
    void *data_destroy_ctx;
};

static DListNode *dlist_node_create(void *data)
{
    DListNode *node = (DListNode *)malloc(sizeof(DListNode));

    if (node != NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        node->data = data;
    }

    return node;
}

static void dlist_node_destroy(DListNode *node,
        DataDestroyFunc data_destroy,
        void *data_destroy_ctx)
{
    if (node != NULL)
    {
        node->prev = NULL;
        node->next = NULL;
        if (data_destroy != NULL)
        {
            data_destroy(data_destroy_ctx, node->data);
        }

        free(node);
    }
}

static DListNode *dlist_get_node(DList *thiz, 
        size_t index, int return_last_if_outrange)
{
    DListNode *iter = thiz->head;
    while (iter->next != NULL && index > 0)
    {
        iter = iter->next;
        index--;
    }

    /* node's index is out of range */
    if (index > 0 && !return_last_if_outrange)
    {
        iter = NULL;
    }

    return iter;
}

DList *dlist_create(DataDestroyFunc data_destroy, void *data_destroy_ctx)
{
    DList *thiz = (DList *)malloc(sizeof(DList));

    if (thiz != NULL)
    {
        thiz->head = NULL;
        thiz->length = 0;
        thiz->data_destroy = data_destroy;
        thiz->data_destroy_ctx = data_destroy_ctx;
    }

    return thiz;
}

void dlist_destroy(DList *thiz)
{
    DListNode *iter = thiz->head;
    DListNode *next = NULL;
    
    while (iter != NULL)
    {
        next = iter->next; 
        dlist_node_destroy(iter, thiz->data_destroy, thiz->data_destroy_ctx);
        iter = next;
    } 

    thiz->head = NULL;
    free(thiz);
}

Ret dlist_insert(DList *thiz, size_t index, void *data)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    DListNode *cursor = NULL;
    DListNode *node = NULL;
    
    if ((node =dlist_node_create(data)) == NULL)
    {
        return RET_OOM;
    }

    thiz->length++;

    /* 1. insert when list empty */
    if (thiz->head == NULL)
    {
        thiz->head = node;
        return RET_OK;
    }

    /* find the insert positon */
    cursor = dlist_get_node(thiz, index, 1);

    if (index < thiz->length)
    {
        /* 2. insert at the top of list */
        if (thiz->head == cursor)
        {
            thiz->head = node;
        }
        /* 3. insert at the middle of list */
        else
        {
            node->prev = cursor->prev;
            cursor->prev->next = node;
        }
        node->next = cursor;
        cursor->prev = node;
    }
    /* 4. insert at the end of list */
    else
    {
        cursor->next = node;
        node->prev = cursor;
    }

    return RET_OK;
}

Ret dlist_append(DList *thiz, void *data)
{
    return dlist_insert(thiz, (size_t)-1, data);
}

Ret dlist_prepend(DList *thiz, void *data)
{
    return dlist_insert(thiz, 0, data);
}

Ret dlist_delete(DList *thiz, size_t index)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    DListNode *cursor = dlist_get_node(thiz, index, 0);
    if (cursor != NULL)
    {
        if (cursor == thiz->head)
        {
            thiz->head = cursor->next;
        }

        if (cursor->prev != NULL)
        {
            cursor->prev->next = cursor->next;
        }

        if (cursor->next != NULL)
        {
            cursor->next->prev = cursor->prev;
        }

        dlist_node_destroy(cursor, thiz->data_destroy, thiz->data_destroy_ctx);

        thiz->length--;
    }

    return RET_OK;
}

Ret dlist_set_by_index(DList *thiz, size_t index, void *data)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    DListNode *cursor = dlist_get_node(thiz, index, 0);
    if (cursor == NULL)
    {
        return RET_FAIL;
    }

    cursor->data = data;

    return RET_OK;
}

Ret dlist_get_by_index(DList *thiz, size_t index, void **data)
{
    return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS);

    DListNode *cursor = dlist_get_node(thiz, index, 0);
    if (cursor == NULL)
    {
        return RET_FAIL;
    }

    *data = cursor->data;

    return RET_OK;
}

size_t dlist_length(DList *thiz)
{
    return thiz->length;
}

int dlist_find(DList *thiz, DataCompareFunc compare, void *ctx)
{
    int index = 0;
    DListNode *iter = thiz->head;
    while (iter != NULL)
    {
        if (compare(ctx, iter->data))
        {
            break;
        }
        index++;
        iter = iter->next;
    }

    return index;
}

Ret dlist_foreach(DList *thiz, DataVisitFunc visit, void *ctx)
{
    return_val_if_fail((thiz != NULL) && (visit != NULL), RET_INVALID_PARAMS);

    Ret ret = RET_OK;
    DListNode *iter = thiz->head;

    while (iter != NULL && ret != RET_STOP)
    {
        ret = visit(ctx, iter->data);
        iter = iter->next;
    }

    return ret;
}

