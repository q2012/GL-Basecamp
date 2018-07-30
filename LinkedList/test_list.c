#include "list.h"

#include <stdio.h>

#ifdef DEBUG
    #include <assert.h>
    #define check(expr) assert((expr))
#else
    #define check(expr) 
#endif

struct test {
    int a;
    struct list list2,list;
};

static inline void print(struct list *el)
{
    printf("%d ", list_entry(el, struct test, list)->a);
}

static inline int cmp_test_list(void *el1, struct list *el2)
{
    return list_entry(el1,struct test, list)->a - list_entry(el2,struct test, list)->a;
}

static inline int cmp_test_list_sort(struct list *el1, struct list *el2)
{
    return cmp_test_list((void *)el1, el2);
}

static inline int count_tens(struct list *el)
{
    return list_entry(el, struct test, list)->a - 10;
}

int main()
{
    printf("\n____________________________\n");
    printf("Create 2 lists.\n");

    CREATE_LIST(test_list);
    CREATE_LIST(test_list1);

    check(test_list.next == test_list.prev && test_list.next == &test_list);
    check(test_list1.next == test_list1.prev && test_list1.next == &test_list1);

    printf("Lists created succesfully");
    printf("\n____________________________\n");
    
    struct test a, a1, a2, a3, a4, a5, a6, a7, a8;
    struct list *temp;

    a.a = 0;
    a1.a = 10;
    a2.a = 20;
    a3.a = 30;
    a4.a = 40;
    a5.a = 50;
    a6.a = 60;
    a7.a = 70;
    a8.a = 80;

    printf("Different adding possibilities\n");

    add_elem(&test_list, &a1.list);
    check(test_list.next == &a1.list && test_list.prev == &a1.list && 
        a1.list.next == &test_list && a1.list.prev == &test_list);

    add_elem_head(&test_list, &a.list);
    check(test_list.next == &a.list && a.list.prev == &test_list &&
        a.list.next == &a1.list && a1.list.prev == &a.list);

    insert_before(&test_list, &a4.list);
    check(test_list.prev == &a4.list && a4.list.next == &test_list &&
        a4.list.prev == &a1.list && a1.list.next == &a4.list);

    insert_after(&a1.list, &a3.list);
    check(a3.list.next == &a4.list && a4.list.prev == &a3.list &&
        a3.list.prev == &a1.list && a1.list.next == &a3.list);

    insert_n(&test_list, &a2.list, 2);
    check(a1.list.next == &a2.list && a2.list.prev == &a1.list &&
        a3.list.prev == &a2.list && a2.list.next == &a3.list);

    insert_n(&test_list, &a5.list, -7);
    check(a5.list.next == &test_list && test_list.prev == &a5.list &&
        a5.list.prev == &a4.list && a4.list.next == &a5.list);

    enum errors error = insert_n_check(&test_list, &a6.list, -8);
    check(a5.list.next == &test_list && test_list.prev == &a5.list &&
        a5.list.prev == &a4.list && a4.list.next == &a5.list && error == INDEX_OUT_OF_BOUNDS);

    error = insert_n_check(&test_list, &a6.list, -1);
    check(a6.list.next == &test_list && test_list.prev == &a6.list &&
        a6.list.prev == &a5.list && a5.list.next == &a6.list && error == OK);
    
    printf("\nWhat we have now: \n");
    list_for_each(temp, &test_list) {
        printf("%d ", list_entry(temp, struct test, list)->a);
    }
    printf("\n____________________________\n");
    printf("Removing possibilities\n");

    delete_list_entry(&a.list);
    check(test_list.next == &a1.list && a1.list.prev == &test_list &&
        a.list.prev == NULL && a.list.next == NULL);
    clear(&a1.list, &a2.list);
    check(test_list.next == &a3.list && a3.list.prev == &test_list &&
        a1.list.prev == NULL && a1.list.next == NULL &&
        a2.list.prev == NULL && a2.list.next == NULL);
    clear_safe(&test_list, &a6.list, &a3.list);
    check(test_list.next == &a4.list && a4.list.prev == &test_list &&
        a5.list.next == &test_list && test_list.prev == &a5.list &&
        a6.list.prev == NULL && a6.list.next == NULL &&
        a3.list.prev == NULL && a3.list.next == NULL);

    clear_all(&test_list);
    check(test_list.next == &test_list && test_list.prev == &test_list &&
        a4.list.prev == NULL && a4.list.next == NULL &&
        a5.list.prev == NULL && a5.list.next == NULL);

    printf("\n____________________________\n");
    add_elem(&test_list, &a.list);
    add_elem(&test_list, &a1.list);
    add_elem(&test_list, &a2.list);
    add_elem(&test_list, &a3.list);
    add_elem(&test_list, &a4.list);
    add_elem(&test_list, &a5.list);
    add_elem(&test_list, &a6.list);
    add_elem(&test_list, &a7.list);
    add_elem(&test_list, &a8.list);

    printf("Traversing possibilities\n");
    traverse(&test_list, print);
    printf("\n");
    list_for_each_reverse(temp, &test_list) {
        printf("%d ", list_entry(temp, struct test, list)->a);
    }
    printf("\n");
    list_for_each_bounds(temp, &a3.list, a5.list.next) {
        printf("%d ", list_entry(temp, struct test, list)->a);
    }
    printf("\n");
    list_for_each_bounds_reverse(temp, a3.list.prev, &a5.list) {
        printf("%d ", list_entry(temp, struct test, list)->a);
    }

    printf("\n____________________________\n");
    printf("Reverse and swap\n");

    reverse(&test_list);
    check(test_list.prev == &a.list && a.list.prev == &a1.list && 
        a1.list.prev == &a2.list && a2.list.prev == &a3.list && 
        a3.list.prev == &a4.list && a4.list.prev == &a5.list && 
        a5.list.prev == &a6.list && a6.list.prev == &a7.list && 
        a7.list.prev == &a8.list && a8.list.prev == &test_list);
    check(test_list.next == &a8.list && a8.list.next == &a7.list && 
        a7.list.next == &a6.list && a6.list.next == &a5.list && 
        a5.list.next == &a4.list && a4.list.next == &a3.list && 
        a3.list.next == &a2.list && a2.list.next == &a1.list && 
        a1.list.next == &a.list && a.list.next == &test_list);

    reverse(&test_list);
    swap_list(&a1.list, &a2.list);
    check(a.list.next == &a2.list && a3.list.prev == &a1.list &&
        a1.list.prev == &a2.list && a2.list.next == &a1.list);
    swap_list(&a2.list, &a1.list);

    printf("\n____________________________\n");
    printf("Count\n");

    check(count_one(&test_list, count_tens) == 1);
    check(count(&test_list, (void *)&a.list, cmp_test_list) == 1);

    printf("\n____________________________\n");
    printf("Sort\n");

    reverse(&test_list);
    sort(&test_list, cmp_test_list_sort, true);

    list_for_each(temp, &test_list) {        printf("%d ", list_entry(temp, struct test, list)->a);    }

    return 0;
}