#include "list.h"

#include <stdint.h>
#include <stdbool.h>

enum errors insert_n_check(struct list *list, struct list *elem, int n)
{
    struct list *temp;
    for(temp = list->next; n > 0 && temp != list; --n)
        temp = temp->next;
    temp = temp->prev;
    for(temp = temp->prev; n < 0 && temp != list; ++n)
        temp = temp->prev;
    temp = temp->next;
    if(n) {
        return INDEX_OUT_OF_BOUNDS;
    }
    __add_elem_middle(temp, elem, temp->next);
    return OK;
}

void traverse(struct list *list, void (*func)(struct list *elem))
{
    struct list *temp;
    list_for_each(temp, list) {
        func(temp);
    }
}

void clear(struct list *from, struct list *to)
{
    if(from == to) return;
    struct list *temp;
    list_for_each_bounds(temp, from->next, to->next) {
        delete_list_entry(temp->prev);
    }
    delete_list_entry(to);
}

void clear_safe(struct list *list, struct list *from, struct list *to)
{
    if(from == to) return;
    struct list *temp;
    list_for_each_bounds(temp, from->next, to->next) {
        if(temp->prev == list) continue;
        delete_list_entry(temp->prev);
    }
    delete_list_entry(to);
}

void clear_all(struct list *list)
{
    struct list *temp;
    list_for_each(temp, list->next) {
        delete_list_entry(temp->prev);
    }
    INIT_LIST(list);
}

/**
 * __swap() - custom swap pointer values without 3-rd variable
 * @x: first value to swap
 * @y: second value to swap
 */
static inline void __swap(struct list **x, struct list **y)
{
    //These casts are annoying, but if i write smth like
    //swap(uintptr_t *x, uintptr_t * y) { *x ^= *y ^= *x ^= *y; }
    //There will be tons of warnings
    *x = (struct list *)((uintptr_t)*x ^ (uintptr_t)*y);
    *y = (struct list *)((uintptr_t)*y ^ (uintptr_t)*x);
    *x = (struct list *)((uintptr_t)*x ^ (uintptr_t)*y);
}

void swap_list(struct list *el1, struct list *el2)
{
    if(el1 == el2) return;
    if(el1->next == el2)
    {
        el1->next = el2->next;
        el2->prev = el1->prev;
        el1->prev = el1->prev->next = el2;
        el2->next = el2->next->prev = el1;
    }
    else if(el2->next == el1)
    {
        el2->next = el1->next;
        el1->prev = el2->prev;
        el2->prev = el2->prev->next = el1;
        el1->next = el1->next->prev = el2;
    }
    else
    {
        el1->prev->next = el1->next->prev = el2;
        el2->prev->next = el2->next->prev = el1;
        __swap(&el1->next, &el2->next);
        __swap(&el1->prev, &el2->prev);
    }
}

void reverse(struct list *list)
{
    struct list *temp;
    list_for_each(temp, list->next) {
        __swap(&(temp->prev)->next, &(temp->prev)->prev);
    }
    __swap(&list->next, &list->prev);
}

int count_one(struct list *list, int (*comp)(struct list *el2))
{
    struct list *temp;
    int res = 0;
    list_for_each(temp, list) {
        res = comp(temp)?res:res + 1;
    }
    return res;
}

int count(struct list *list, void *val, int (*comp)(void *val, struct list *el2))
{
    struct list *temp;
    int res = 0;
    list_for_each(temp, list) {
        res = comp(val, temp)?res:res + 1;
    }
    return res;
}

/**
 * Implements partition of quicksort
 */
static struct list * __qs_partition(struct list **lo, struct list **hi, int (*comp)(struct list *el1, struct list *el2))
{
    struct list *pivot = *hi;
    struct list *i = (*lo)->prev;
    struct list *j;

    list_for_each_bounds(j,*lo,*hi) {
        if(comp(j, pivot) <= 0)
        {
            i = i->next;
            if(i == *lo)
                *lo = j;
            swap_list(i,j);
            __swap(&i, &j);
        }
    }
    i = i->next;
    
    *hi = i;
    if(i == *lo)
        *lo = pivot;
        
    swap_list(i, pivot);
    return pivot;
}

/**
 * Main sorting function of quicksort
 */
static void __qsort(struct list *list, struct list *lo, struct list *hi, int (*comp)(struct list *el1, struct list *el2))
{
    if(lo != hi && hi->next != lo && lo->prev != hi)
    {
        struct list *pi = __qs_partition(&lo, &hi, comp);
        
        __qsort(list, lo, pi->prev, comp);
        __qsort(list, pi->next, hi, comp);
    }
}

void sort(struct list *list, int (*comp)(struct list *el1, struct list *el2), bool order)
{
    __qsort(list, list->next, list->prev, comp);
    if(!order) reverse(list);
}