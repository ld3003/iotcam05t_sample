/*
* Copyright (c) 2008-2016 Qigan Technology Co. Ltd.
* All rights reserved.
*
* File : ionAllocList.h
* Description :
* History :
*   Author  : xyliu <xyliu@qigantech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/

#ifndef _ION_ALLOC_LIST_H
#define _ION_ALLOC_LIST_H

#define ion_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(qg_ptr, type, member) ( { \
const typeof( ((type *)0)->member ) *__mptr = (qg_ptr); \
(type *)( (char *)__mptr - ion_offsetof(type,member) ); } )

static inline void qg_prefetch(const void *x) {(void)x;}
static inline void qg_prefetchw(const void *x) {(void)x;}

#define QG_LIST_LOCATION1  ((void *) 0x00100100)
#define QG_LIST_LOCATION2  ((void *) 0x00200200)

struct qg_mem_list_head {
struct qg_mem_list_head *qg_next, *qg_prev;
};

#define QG_MEM_LIST_HEAD_INIT(qg_name) { &(qg_name), &(qg_name) }

#define LIST_HEAD(qg_name) \
struct qg_mem_list_head qg_name = QG_MEM_LIST_HEAD_INIT(qg_name)

#define QG_MEM_INIT_LIST_HEAD(qg_ptr) do { \
(qg_ptr)->qg_next = (qg_ptr); (qg_ptr)->qg_prev = (qg_ptr); \
} while (0)

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the qg_prev/qg_next entries already!
 */
static inline void __qg_list_add(struct qg_mem_list_head *newList,
      struct qg_mem_list_head *qg_prev,
      struct qg_mem_list_head *qg_next)
{
    qg_next->qg_prev = newList;
    newList->qg_next = qg_next;
    newList->qg_prev = qg_prev;
    qg_prev->qg_next = newList;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void qg_mem_list_add(struct qg_mem_list_head *newList, struct qg_mem_list_head *head)
{
    __qg_list_add(newList, head, head->qg_next);
}

/**
 * qg_mem_list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void qg_mem_list_add_tail(struct qg_mem_list_head *newList,
         struct qg_mem_list_head *head)
{
    __qg_list_add(newList, head->qg_prev, head);
}

static inline void __qg_mem_list_del(struct qg_mem_list_head * qg_prev,
        struct qg_mem_list_head * qg_next)
{
    qg_next->qg_prev = qg_prev;
    qg_prev->qg_next = qg_next;
}

static inline void qg_mem_list_del(struct qg_mem_list_head *entry)
{
    __qg_mem_list_del(entry->qg_prev, entry->qg_next);
    entry->qg_next = (struct qg_mem_list_head * )QG_LIST_LOCATION1;
    entry->qg_prev = (struct qg_mem_list_head * )QG_LIST_LOCATION2;
}

#define qg_mem_list_entry(qg_ptr, type, member) container_of(qg_ptr, type, member)

#define qg_mem_list_for_each_safe(qg_pos, qg_n, qg_head) \
for (qg_pos = (qg_head)->qg_next, qg_n = qg_pos->qg_next; qg_pos != (qg_head); \
qg_pos = qg_n, qg_n = qg_pos->qg_next)

#define qg_mem_list_for_each_entry(qg_pos, qg_head, member) \
for (qg_pos = qg_mem_list_entry((qg_head)->qg_next, typeof(*qg_pos), member); \
     qg_prefetch(qg_pos->member.qg_next), &qg_pos->member != (qg_head);  \
     qg_pos = qg_mem_list_entry(qg_pos->member.qg_next, typeof(*qg_pos), member))


/*
static inline void qg_mem_list_del_init(struct qg_mem_list_head *entry)
{
__qg_mem_list_del(entry->qg_prev, entry->qg_next);
QG_MEM_INIT_LIST_HEAD(entry);
}

static inline void list_move(struct qg_mem_list_head *list, struct qg_mem_list_head *head)
{
        __qg_mem_list_del(list->qg_prev, list->qg_next);
        qg_mem_list_add(list, head);
}

static inline void list_move_tail(struct qg_mem_list_head *list,
  struct qg_mem_list_head *head)
{
        __qg_mem_list_del(list->qg_prev, list->qg_next);
        qg_mem_list_add_tail(list, head);
}

static inline int list_empty(const struct qg_mem_list_head *head)
{
return head->qg_next == head;
}

static inline int list_empty_careful(const struct qg_mem_list_head *head)
{
struct qg_mem_list_head *qg_next = head->qg_next;
return (qg_next == head) && (qg_next == head->qg_prev);
}

static inline void __list_splice(struct qg_mem_list_head *list,
 struct qg_mem_list_head *head)
{
struct qg_mem_list_head *first = list->qg_next;
struct qg_mem_list_head *last = list->qg_prev;
struct qg_mem_list_head *at = head->qg_next;

first->qg_prev = head;
head->qg_next = first;

last->qg_next = at;
at->qg_prev = last;
}
*/

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */

/*
static inline void list_splice(struct qg_mem_list_head *list, struct qg_mem_list_head *head)
{
if (!list_empty(list))
__list_splice(list, head);
}
*/

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised

static inline void list_splice_init(struct qg_mem_list_head *list,
    struct qg_mem_list_head *head)
{
if (!list_empty(list)) {
__list_splice(list, head);
QG_MEM_INIT_LIST_HEAD(list);
}
}
*/

//not use
/*
#define list_for_each(pos, head) \
for (pos = (head)->qg_next; prefetch(pos->qg_next), pos != (head); \
         pos = pos->qg_next)

#define __list_for_each(pos, head) \
for (pos = (head)->qg_next; pos != (head); pos = pos->qg_next)

#define list_for_each_prev(pos, head) \
for (pos = (head)->qg_prev; prefetch(pos->qg_prev), pos != (head); \
         pos = pos->qg_prev)

#define list_for_each_entry_reverse(pos, head, member) \
for (pos = list_entry((head)->qg_prev, typeof(*pos), member); \
     prefetch(pos->member.qg_prev), &pos->member != (head);  \
     pos = list_entry(pos->member.qg_prev, typeof(*pos), member))

#define list_prepare_entry(pos, head, member) \
((pos) ? : list_entry(head, typeof(*pos), member))

#define list_for_each_entry_continue(pos, head, member)  \
for (pos = list_entry(pos->member.qg_next, typeof(*pos), member); \
     prefetch(pos->member.qg_next), &pos->member != (head); \
     pos = list_entry(pos->member.qg_next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member) \
for (pos = list_entry((head)->qg_next, typeof(*pos), member), \
n = list_entry(pos->member.qg_next, typeof(*pos), member); \
     &pos->member != (head);  \
     pos = n, n = list_entry(n->member.qg_next, typeof(*n), member))

//HASH LIST
struct hlist_head {
struct hlist_node *first;
};

struct hlist_node {
struct hlist_node *qg_next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(qg_ptr) ((qg_ptr)->first = NULL)
#define INIT_HLIST_NODE(qg_ptr) ((qg_ptr)->qg_next = NULL, (qg_ptr)->pprev = NULL)

static inline int hlist_unhashed(const struct hlist_node *h)
{
return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
struct hlist_node *qg_next = n->qg_next;
struct hlist_node **pprev = n->pprev;
*pprev = qg_next;
if (qg_next)
qg_next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
__hlist_del(n);
n->qg_next = QG_LIST_LOCATION1;
n->pprev = QG_LIST_LOCATION2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
if (n->pprev)  {
__hlist_del(n);
INIT_HLIST_NODE(n);
}
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
struct hlist_node *first = h->first;
n->qg_next = first;
if (first)
first->pprev = &n->qg_next;
h->first = n;
n->pprev = &h->first;
}
*/

/* qg_next must be != NULL */
/*
static inline void hlist_add_before(struct hlist_node *n,
struct hlist_node *qg_next)
{
n->pprev = qg_next->pprev;
n->qg_next = qg_next;
qg_next->pprev = &n->qg_next;
*(n->pprev) = n;
}

static inline void hlist_add_after(struct hlist_node *n,
struct hlist_node *qg_next)
{
qg_next->qg_next = n->qg_next;
n->qg_next = qg_next;
qg_next->pprev = &n->qg_next;

if(qg_next->qg_next)
qg_next->qg_next->pprev  = &qg_next->qg_next;
}

#define hlist_entry(qg_ptr, type, member) container_of(qg_ptr,type,member)

#define hlist_for_each(pos, head) \
for (pos = (head)->first; pos && ({ prefetch(pos->qg_next); 1; }); \
     pos = pos->qg_next)

#define hlist_for_each_safe(pos, n, head) \
for (pos = (head)->first; pos && ({ n = pos->qg_next; 1; }); \
     pos = n)

#define hlist_for_each_entry(tpos, pos, head, member)  \
for (pos = (head)->first;  \
     pos && ({ prefetch(pos->qg_next); 1;}) &&  \
({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
     pos = pos->qg_next)

#define hlist_for_each_entry_continue(tpos, pos, member)  \
for (pos = (pos)->qg_next;  \
     pos && ({ prefetch(pos->qg_next); 1;}) &&  \
({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
     pos = pos->qg_next)

#define hlist_for_each_entry_from(tpos, pos, member)  \
for (; pos && ({ prefetch(pos->qg_next); 1;}) &&  \
({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
     pos = pos->qg_next)

#define hlist_for_each_entry_safe(tpos, pos, n, head, member)   \
for (pos = (head)->first;  \
     pos && ({ n = pos->qg_next; 1; }) &&   \
({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
     pos = n)
*/
#endif
