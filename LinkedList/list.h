#include <stdbool.h>
#include <stddef.h>

/**
 * enum errors - errors
 * @OK: no errors
 * @INDEX_OUT_OF_BOUNDS: index out of bounds
 *
 * Now they used only in insert_n_check() method.
 */
enum errors {
    OK=0,
    INDEX_OUT_OF_BOUNDS
};

/**
 * struct list - double-linked list node. Designed to be part of data struct.
 * @prev: pointer to previous list node
 * @next: pointer to next list node
 */
struct list {
    struct list *prev, *next;
};

/**
 * INIT_LIST(l) - Create list from existing list node
 * @l: pointer to existing node
 */
#define INIT_LIST(l) do { \
        (l)->next = (l); (l)->prev = (l); \
        } while(0)
        
/**
 * INIT_LIST_HEAD(name) - Initialization cortege for CREATE_LIST
 */
#define INIT_LIST_HEAD(name) {&(name), &(name)}

/**
 * CREATE_LIST(name) - Create list node, which use must be similar to "list object". 
 * It represents the list itself.
 */
#define CREATE_LIST(name) struct list name = INIT_LIST_HEAD(name)

/**
 * list_entry(ptr, type, member) - Get pointer to parent struct.
 * @ptr: pointer to list node
 * @type: type of parent struct
 * @member: name of list node in parent struct
 */
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each(elem, list) - Macro for manual iterating list
 * @elem: pointer to current list node. Has to be pre-created as struct list *
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 */
#define list_for_each(elem, list) for(elem = (list)->next; elem != (list); \
                                        elem = elem->next)

/**
 * list_for_each_reverse(elem, list) - Macro for manual iterating list in reverse order
 * @elem: pointer to current list node. Has to be pre-created as struct list *
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 */
#define list_for_each_reverse(elem, list) for(elem = (list)->prev; elem != (list); \
                                        elem = elem->prev)

/**
 * list_for_each_bounds(elem, lo, hi) - Macro for manual iterating list
 * @elem: pointer to current list node. Has to be pre-created as struct list *
 * @lo: pointer to start list node
 * @hi: pointer to end list node
 */
#define list_for_each_bounds(elem, lo, hi) \
                for(elem = (lo); elem != (hi); elem = elem->next)

/**
 * list_for_each_bounds_reverse(elem, lo, hi) - Macro for manual iterating list in reverse order
 * @elem: pointer to current list node. Has to be pre-created as struct list *
 * @hi: pointer to start list node
 * @lo: pointer to end list node
 */
#define list_for_each_bounds_reverse(elem, hi, lo) \
                for(elem = (lo); elem != (hi); elem = elem->prev)

/**
 * list_for_each_safe(elem, temp, list) - Macro for manual iterating list safe against removal
 * @elem: pointer to current list node. Has to be pre-created as struct list *
 * @temp: temporary storage. Has to be pre-created as struct list *
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 */
#define list_for_each_safe(elem, temp, list) \
        for(elem = (list)->next, temp = elem->next; \
                elem != (list);elem = temp, temp = elem->next)

/**
 * __add_elem_middle() - Add element between two consecutive nodes(prev and next). 
 * @prev: pointer to node to be before element
 * @elem: pointer to list node to insert
 * @next: pointer to node to be after element
 */
static inline void __add_elem_middle(struct list *prev, struct list *elem, struct list *next)
{
    prev->next = elem;
    next->prev = elem;
    elem->next = next;
    elem->prev = prev;
}

/**
 * add_elem() - add elem to the tail of list
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @elem: pointer to list node to be added
 */
#define add_elem(list, elem) insert_before((list), (elem))

/**
 * add_elem_head() - add elem to the head of list
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @elem: pointer to list node to be added
 */
#define add_elem_head(list, elem) insert_after((list), (elem))


/**
 * insert_before() - add elem before other
 * @next: pointer to other element.
 * @elem: pointer to list node to be added before next
 */
static inline void insert_before(struct list *next, struct list *elem)
{
    __add_elem_middle(next->prev, elem, next);
}

/**
 * insert_after() - add elem after other
 * @prev: pointer to other element.
 * @elem: pointer to list node to be added after next
 */
static inline void insert_after(struct list *prev, struct list *elem)
{
    __add_elem_middle(prev, elem, prev->next);
}

/**
 * insert_n() - add elem n far away from current.
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @elem: pointer to list node to be added
 * @n: How far from list we need to insert elem. Negative value means another direction
 *
 * No n check provided. If n is greater than list size - elem will be inserted as if
 * n was (n - size - 1), if less than negative list_size(-list_size) - (n + size + 1) and so on.
 * 
 * I left this func here just to illustrate problem.
 * 
 * IMPORTANT NOT FOR LIB BUT FOR IMPLEMENTATION
 * I create pointer to struct list. In kernel lists in functions they didn`t need it, 
 * only in macro, and there they imported it like list_for_each_safe(pos, temp, head)
 * While in macro they don`t actually have other approach, here i can go either way.
 * Just for demonstration, how this could look like if i import temp, here is the code.
 * Also it seems like if i create inner variable, compiler won`t make this func as inline
 * (i`m not very sure of it). This is first time in this file this dilemma appears, 
 * so after this explanation i won`t touch it anymore.
 * 
 * * static inline void insert_n(struct list *list, struct list *elem, int n, struct list *temp)
 * * {
 * *     for(temp = list;n > 0; --n)
 * *         temp = temp->next;
 * *     for(;n < 0; ++n)
 * *         temp = temp->prev;
 * *     __add_elem_middle(temp, elem, temp->next);
 * *     temp = NULL;
 * * }
 * 
 */
static inline void insert_n(struct list *list, struct list *elem, int n)
{
    struct list *temp;
    for(temp = list;n > 0; --n)
        temp = temp->next;
    for(;n < 0; ++n)
        temp = temp->prev;
    __add_elem_middle(temp, elem, temp->next);
}

/**
 * insert_n_check() - add elem n far away from current.
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @elem: pointer to list node to be added
 * @n: How far from list we need to insert elem. Negative value means another direction
 * 
 * If n is greater than list size, it will produce error.
 *
 * Return:
 * * OK - inserted
 * * INDEX_OUT_OF_BOUNDS - not inserted
 */
enum errors insert_n_check(struct list *list, struct list *elem, int n);

/**
 * __remove_elem() - removes elem from list, but not deletes it.
 * @elem: elem to be removed
 *
 * After this operation elem will save his pointers, but won`t be accesible from list.
 */
static inline void __remove_elem(struct list *elem)
{
    elem->next->prev = elem->prev;
    elem->prev->next = elem->next;
}


/**
 * delete_list_entry() - deletes node from list and nullifies list node pointers.
 * @elem: elem to be deleted
 */
static inline void delete_list_entry(struct list *elem)
{
    __remove_elem(elem);
    elem->next = elem->prev = NULL;
}

/**
 * clear() - delete list nodes in bounds [from; to]
 * @from: start node
 * @to: end node
 *
 * Iterating goes through next pointer. This is not safe if parent node is in range.
 */
void clear(struct list *from, struct list *to);

/**
 * clear_safe() - delete node in range [from; to], always safe
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @from: start node
 * @to: end node
 */
void clear_safe(struct list *list, struct list *from, struct list *to);

/**
 * clear_all() - delete all nodes in list.
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 */
void clear_all(struct list *list);

/**
 * traverse() - traverse through list using custom func.
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @func: function to be called for every list node
 * 
 * This func is not safe against deleting/modifying temp in func.
 */
void traverse(struct list *list, void (*func)(struct list *elem));

/**
 * swap_list() - swap list nodes without 3-rd variable
 * @el1: first list node to swap
 * @el2: second list node to swap
 */
void swap_list(struct list *el1, struct list *el2);

/**
 * reverse() - reverse order of list
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 */
void reverse(struct list *list);

/**
 * count_one() - count values using comp as comparator
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @comp(): function will be used to compare values.
 *
 * comp() has to return 0 if value given equals to some ideal value.
 *
 * Return: number of elements, equal to ideal
 */
int count_one(struct list *list, int (*comp)(struct list *el2));

/**
 * count() - count values using comp as comparator
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @val: pointer to data, which will be compared
 * @comp(): function will be used to compare values.
 *
 * comp() has to return 0 if value given are equal
 *
 * Return: number of elements, equal to given
 */
int count(struct list *list, void *val, int (*comp)(void *val, struct list *el2));

/**
 * sort() - sort list
 * @list: pointer to parent list node. E.g. created with CREATE_LIST
 * @comp(): function used as comparator.
 * @order: true - ascendeng, false - descending
 *
 * comp has to return 0 if equals, >0 if el1 > el2, <0 if el2 > el1
 */
void sort(struct list *list, int (*comp)(struct list *el1, struct list *el2), bool order);