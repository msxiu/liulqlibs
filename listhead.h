/**链表头是原理是通过把
a.next->b b.next->c c.next->a
a.prev->c c.prev->b b.prev->a
这样循环链接方式实现
*/
#ifndef LIST_HEAD_H_
#define LIST_HEAD_H_

#if defined(WIN32)
#define INLINE __inline
#else
#define INLINE inline
#endif


//链表头结构体
typedef struct listhead_struct {
	struct listhead_struct *next;//下一个结节
	struct listhead_struct *prev;//上一个结点
} listhead_t, *listhead_p;

//初始化链表头结构
#ifndef LIST_HEAD_INIT
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#endif

//申请链表头节点,
#ifndef LIST_HEADM
#define LIST_HEADM(name) \
    listhead_t name = LIST_HEAD_INIT(name)
#endif

//链表头结构指针赋值
#ifndef INIT_LIST_HEAD
#define INIT_LIST_HEAD(ptr)  do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)
#endif


static INLINE void __listhead_add(listhead_t *new_n, listhead_t *prev, listhead_t *next) {
    next->prev = new_n;
    new_n->next = next;
    new_n->prev = prev;
    prev->next = new_n;
}

//添加节点到链表头
static INLINE void listhead_add_n(listhead_t *new_n, listhead_t *head) {
    __listhead_add(new_n, head, head->next);
}

//添加节点到锭表尾
static INLINE void listhead_add_tail(listhead_t *new_n, listhead_t *head) {
    __listhead_add(new_n, head->prev, head);
}


static INLINE void __listhead_del(listhead_t *prev, listhead_t *next) {
    next->prev = prev;
    prev->next = next;
}

//删除一个节点
static INLINE void listhead_del(listhead_t *entry) {
    __listhead_del(entry->prev, entry->next);
    entry->next = (listhead_t*)(void *) 0;
    entry->prev = (listhead_t*)(void *) 0;
}

/** listhead_del_init - 从链表中删除节点,并以该实体初始化链表. */
static INLINE void listhead_del_init(listhead_t *entry) {
    __listhead_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * listhead_move - 从一个列表中删除，并添加作为另一个的头部
 * @list: 删除的节点
 * @head: 要添加的头节点
 */
static INLINE void listhead_move(listhead_t *list, listhead_t *head) {
    __listhead_del(list->prev, list->next);
    listhead_add_n(list, head);
}

/**
 * listhead_move - 从一个列表中删除，并添加作为另一个的尾部
 * @list: 删除的节点
 * @head: 要添加的头节点
 */
static INLINE void listhead_move_tail(listhead_t *list, listhead_t *head) {
    __listhead_del(list->prev, list->next);
    listhead_add_tail(list, head);
}

static INLINE int listhead_empty(listhead_t *head) {//测试列表是否为空
    return head->next == head;
}

static INLINE void __listhead_splice(listhead_t *list, listhead_t *head) {
    listhead_t *first = list->next;
    listhead_t *last = list->prev;
    listhead_t *at = head->next;

    first->prev = head;
    head->next = first;
    last->next = at;
    at->prev = last;
}


static INLINE void listhead_splice(listhead_t *list, listhead_t *head){//连接两个链表
    if (!listhead_empty(list))
        __listhead_splice(list, head);
}

/**
 * listhead_splice_init - 连接两个列表和初始化清空列表.*/
static INLINE void listhead_splice_init(
	listhead_t *list,	//新列表中添加, @list 链表被初始化.
	listhead_t *head	//将其添加在第一个列表.
) {
    if (!listhead_empty(list)) {
        __listhead_splice(list, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * listhead_entry - 获得数据实体
 * @ptr: listhead_head结构体指针.
 * @type: 链表结构体名称.
 * @member: 链表头成员在结构体中的名称.
 */
#define listhead_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * listhead_for_each - 遍历链表
 * @pos: the &listhead_t to use as a loop counter.
 * @head: 链表头.
 */
#define listhead_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * listhead_for_each_prev - 向前遍历链表
 * @pos: the &listhead_t to use as a loop counter.
 * @head: 链表头.
 */
#define listhead_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * listhead_for_each_safe - 遍历链表,预防遍历中删除节点
 * @pos: the &listhead_t to use as a loop counter.
 * @n:  another &listhead_t to use as temporary storage
 * @head: 链表头.
 */
#define listhead_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * listhead_for_each_entry - 遍历链表,这里的pos是数据项结构指针类型，而不是(listhead_t *)
 * @pos: the type * to use as a loop counter.
 * @head: the head for your list.
 * @member: the name of the listhead_struct within the struct.
 */
#define listhead_for_each_entry(pos, head, member)    \
    for (pos = listhead_entry((head)->next, typeof(*pos), member); \
            &pos->member != (head);      \
            pos = listhead_entry(pos->member.next, typeof(*pos), member))

/**
 * listhead_for_each_entry_safe - 遍历链表,预防遍历中删除节点,这里的pos是数据项结构指针类型
 * @pos: the type * to use as a loop counter.
 * @n:  another type * to use as temporary storage
 * @head: the head for your list.
 * @member: the name of the listhead_struct within the struct.
 */
#define listhead_for_each_entry_safe(pos, n, head, member)   \
    for (pos = listhead_entry((head)->next, typeof(*pos), member), \
            n = listhead_entry(pos->member.next, typeof(*pos), member); \
            &pos->member != (head);      \
            pos = n, n = listhead_entry(n->member.next, typeof(*n), member))



#endif /* toplist.h */
