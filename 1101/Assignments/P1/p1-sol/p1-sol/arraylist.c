/* Author: Steffen Viken Valvaag <steffenv@cs.uit.no> */
#include "list.h"

#include <stdlib.h>

struct list
{
    cmpfunc_t cmpfunc;
    void **array;
    int capacity;
    int size;
    int head;
    int tail;
};

struct list_iter
{
    list_t *list;
    int n;
};


list_t *list_create(cmpfunc_t cmpfunc)
{
    list_t *l;
    
    l = malloc(sizeof(list_t));
    if (l == NULL)
        fatal_error("out of memory");
    l->cmpfunc = cmpfunc;
    l->capacity = 4;
    l->array = calloc(l->capacity, sizeof(void *));
    if (l->array == NULL)
        fatal_error("out of memory");
    l->size = 0;
    l->head = 0;
    l->tail = 0;
    return l;
}

void list_destroy(list_t *list)
{
    free(list->array);
    free(list);
}

int list_size(list_t *list)
{
    return list->size;
}

static inline int nextindex(list_t *list, int index)
{
    return (index + 1) % list->capacity;
}

static inline int previndex(list_t *list, int index)
{
    return (index + list->capacity - 1) % list->capacity;
}

/*
 * Returns the nth element of the list.
 */
static inline void *getelem(list_t *list, int n)
{
    return list->array[(list->head + n) % list->capacity];
}

/*
 * Sets the nth element of the list.
 */
 static inline void setelem(list_t *list, int n, void *elem)
 {
     list->array[(list->head + n) % list->capacity] = elem;
 }

/*
 * Resizes the internal array of the list.
 */
static void resizelist(list_t *list, int newcapacity)
{
    void **newarray;
    
    /* Allocate the new array */
    newarray = calloc(newcapacity, sizeof(void *));
    if (newarray == NULL)
        fatal_error("out of memory");
    
    /* Copy the elements to the new array */
    if (list->size > 0) {
        if (list->head < list->tail) {
            /* Simple case: the data is stored contiguously */
            memcpy(newarray, list->array + list->head,
                   list->size * sizeof(void *));
        }
        else {
            /* Harder case: we need to do two copies */
            memcpy(newarray,
                   list->array + list->head,
                   (list->capacity - list->head) * sizeof(void *));
            memcpy(newarray + list->capacity - list->head,
                   list->array,
                   list->tail * sizeof(void *)); 
        }
    }
    
    /* Update the head and tail pointers */
    list->head = 0;
    list->tail = list->size;
    
    /* Free the old array and install the new one */
    free(list->array);
    list->array = newarray;
    list->capacity = newcapacity;
}

void list_addfirst(list_t *list, void *elem)
{
    if (list->size >= list->capacity) {
        resizelist(list, list->capacity * 2);
    }
    list->head = previndex(list, list->head);
    list->array[list->head] = elem;
    list->size++;
}

void list_addlast(list_t *list, void *elem)
{
    if (list->size >= list->capacity) {
        resizelist(list, list->capacity * 2);
    }
    list->array[list->tail] = elem;
    list->tail = nextindex(list, list->tail);
    list->size++;
}

void *list_popfirst(list_t *list)
{
    void *elem;
    
    if (list->size == 0)
        fatal_error("list_popfirst on empty list");
            
    elem = list->array[list->head];
    list->head = nextindex(list, list->head);
    list->size--;
    if (list->capacity > 4 && list->size <= list->capacity / 3) {
        resizelist(list, list->capacity / 2);
    }
    return elem;
}

void *list_poplast(list_t *list)
{
    void *elem;
    
    if (list->size == 0)
        fatal_error("list_poplast on empty list");
    
    list->tail = previndex(list, list->tail);
    elem = list->array[list->tail];
    list->size--;
    if (list->capacity > 4 && list->size <= list->capacity / 3) {
        resizelist(list, list->capacity / 2);
    }
    return elem;
}

int list_contains(list_t *list, void *elem)
{
    int i;
    
    for (i = 0; i < list->size; i++) {
        if (list->cmpfunc(elem, getelem(list, i)) == 0) {
            return 1;            
        }
    }
    return 0;
}

void list_sort(list_t *list)
{
    int n;
    int i, j, min;
    void *tmp;
    
    /* Selection sort */
    n = list->size;
    for (i = 0; i < n; i++) {
        min = i;
        for (j = i+1; j < n; j++) {
            if (list->cmpfunc(getelem(list, j), getelem(list, min)) < 0) {
                min = j;
            }
        }
        tmp = getelem(list, min);
        setelem(list, min, getelem(list, i));
        setelem(list, i, tmp);
    }
}

list_iter_t *list_createiter(list_t *list)
{
    list_iter_t *it = malloc(sizeof(list_iter_t));
    if (it == NULL)
        fatal_error("out of memory");
    it->list = list;
    it->n = 0;
    return it;
}

void list_destroyiter(list_iter_t *iter)
{
    free(iter);
}

int list_hasnext(list_iter_t *iter)
{
    return iter->n < iter->list->size;
}

void *list_next(list_iter_t *iter)
{
    void *elem;
    
    elem = getelem(iter->list, iter->n);
    iter->n++;
    return elem;
}
