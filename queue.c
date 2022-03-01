#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer?????
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q = (struct list_head *) malloc(sizeof(struct list_head));
    if (q != NULL) {
        INIT_LIST_HEAD(q);
    }
    return q;  // If could not allocate space, q is automatically NULL
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    // Since q_free is only called when l != NULL (at least in q_new), the
    // following code might not be necessary.
    if (l == NULL)
        return;
    struct list_head *node = l->next;
    while (node != l) {
        node = node->next;
        q_release_element(list_entry(node->prev, element_t, list));
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *e = (element_t *) malloc(sizeof(element_t));
    if (e == NULL)
        return false;
    e->value = strdup(s);  // strdup already does the malloc, WHICH MIGHT FAIL
    if (e->value == NULL)  // if string memory alloc failed
    {
        free(e);
        return false;
    }
    list_add(&e->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *e = (element_t *) malloc(sizeof(element_t));
    if (e == NULL)
        return false;
    e->value = strdup(s);  // strdup already does the malloc, WHICH MIGHT FAIL
    if (e->value == NULL)  // if string memory alloc failed
    {
        free(e);
        return false;
    }
    list_add_tail(&e->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *e = list_first_entry(head, element_t, list);
    if (sp != NULL) {
        // incorrect: strndup does a new malloc, storing the value to a new
        // address other than where sp is originally pointing to sp =
        // strndup(e->value, bufsize-1);
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&e->list);
    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *e = list_last_entry(head, element_t, list);
    if (sp != NULL) {
        strncpy(sp, e->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&e->list);
    return e;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int i = 0;
    struct list_head *it = head->next;
    while (it != head) {
        it = it->next;
        ++i;
    }
    return i;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
// https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
bool q_delete_mid(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return false;

    struct list_head *l = head->next;
    struct list_head *r = head->prev;

    while (l != r && l->next != r) {
        l = l->next;
        r = r->prev;
    }

    r = l;
    list_del(r);
    q_release_element(list_entry(l, element_t, list));

    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function will always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // see HackMD quiz1-2 and test them

    if (!head)
        return false;

    struct list_head **curr = &head->next;
    struct list_head *tmp;
    while ((*curr) != head) {
        if ((*curr)->next == head ||
            strcmp(
                list_entry((*curr), element_t, list)->value,
                list_entry((*curr)->next, element_t, list)->value)) {  // diff
            curr = &(*curr)->next;
        } else {
            printf("%s is a dup\n",
                   list_entry((*curr), element_t, list)->value);
            do {
                tmp = *curr;
                (*curr)->next->prev = (*curr)->prev;  // !!!IMPORTANT!!!
                *curr = (*curr)->next;
                q_release_element(list_entry(tmp, element_t, list));
            } while ((*curr)->next != head &&
                     !strcmp(list_entry((*curr), element_t, list)->value,
                             list_entry((*curr)->next, element_t, list)
                                 ->value));  // same
            tmp = *curr;
            (*curr)->next->prev = (*curr)->prev;  // !!!IMPORTANT!!!
            *curr = (*curr)->next;
            (*curr)->next->prev = *curr;
            q_release_element(list_entry(tmp, element_t, list));
        }
    }

    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
// https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *left = head->next;
    struct list_head *right = left->next;

    while (left != head && right != head) {
        /* Don't modify the "outwards" pointers first, e.g. left->prev or
         * right->next \
         * because they are the only variable giving us reference to the left
         * neightbor \ and the right neighbor. */

        /* Make the neighbors point to the correct new nodes */
        right->next->prev = left;
        left->prev->next = right;

        /* Make the two nodes point to the new neighbors */
        right->prev = left->prev;
        left->next = right->next;

        /* Change outward pointers */
        right->next = left;
        left->prev = right;

        /* Next two pairs */
        left = left->next;
        right = left->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return;
    struct list_head *it = head;
    struct list_head *tmp;

    do {
        tmp = it->next;
        it->next = it->prev;
        it->prev = tmp;
        it = it->prev; /* it = it->next works too */
    } while (it != head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head) {}
