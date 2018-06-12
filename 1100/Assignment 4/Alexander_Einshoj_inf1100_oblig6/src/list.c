#include <stdio.h>
#include <stdlib.h>
#include "list.h"


/*
 * List implementation
 */
struct listnode;

typedef struct listnode listnode_t;

struct listnode {
    listnode_t *next;
    listnode_t *prev;
    void *elem;
};

struct list {
    listnode_t *head;
    listnode_t *tail;
    int size;
};

void fatal_error(char *msg)
{
    fprintf(stderr, "fatal error: %s\n", msg);
    exit(EXIT_FAILURE);
}

static listnode_t *newnode(void *elem)
{
    listnode_t *node = malloc(sizeof(listnode_t));
    if (node == NULL)
        fatal_error("out of memory");
    node->next = NULL;
    node->prev = NULL;
    node->elem = elem;
    return node;
}

// Create new list
list_t *list_create(void)
{
    list_t *list = malloc(sizeof(list_t));
    if (list == NULL)
        fatal_error("out of memory");
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

// Free list. items not freed.
void list_destroy(list_t *list)
{
    listnode_t *node = list->head;
    while (node != NULL) {
        listnode_t *tmp = node;
        node = node->next;
        free(tmp);
    }
    free(list);
}

// Insert item first in list
void list_addfirst(list_t *list, void *elem)
{
    listnode_t *node = newnode(elem);
    if (list->head == NULL) {
        list->head = list->tail = node;
    }
    else {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    }
    list->size++; 
}


// Insert item last in list.
void list_addlast(list_t *list, void *elem)
{
    listnode_t *node = newnode(elem);
    if (list->head == NULL) {
        list->head = list->tail = node;
    }
    else {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
    list->size++;
}



// Remove item from list
void list_remove(list_t *list, void *elem)
{
    listnode_t *node = list->head;
    if(list->size == 0)
        return;
    if(node->next == NULL)
    {
        list->head = NULL;    
        free(node);
        list->size--;
        return;
    }
    while (node->next != NULL)
    {
        if(node->elem == elem)
        {
            if(node->prev != NULL)
            {
                node->prev->next = node->next;
                node->next->prev = node->prev;
            }
            else
            {
                list->head = node->next;
                list->head->prev = NULL;
            }
            free(node);
            list->size--;
            return;
        }
        else
            node = node->next;

    }
}


// Return # of items in list
int list_size(list_t *list)
{
    return list->size;
}



/*
 * Iterator implementation
 */
 

struct list_iterator {
    listnode_t *node;
};


// Create new list iterator
list_iterator_t *list_createiterator(list_t *list)
{
    list_iterator_t *iter = malloc(sizeof(list_iterator_t));
    if (iter == NULL)
        fatal_error("out of memory");
    iter->node = list->head;
    return iter;
}




// Free iterator
void list_destroyiterator(list_iterator_t *iter)
{
    free(iter);
}

// Move iterator to next item in list and return current.
void *list_next(list_iterator_t *iter)
{
    if (iter->node == NULL) {
        return NULL;
    }
    else {
        void *elem = iter->node->elem;
        iter->node = iter->node->next;
        return elem;
    }
}


// Let iterator point to first item in list again
/*void list_resetiterator(list_iterator_t *iter)
{
        void *elem = iter->node->elem;
        while(iter->node->prev != NULL)
            iter->node = iter->node->prev;
        elem = iter->node->prev;
        iter->node->next = iter->node;
}*/




