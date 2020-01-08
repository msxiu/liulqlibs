/** 使用等待线程获取资源链，当没有数据处理时，线程进入等待状态
 * void linkstack_initialize(linkstack_p o);//1:初始化
 * void linkstack_push(threadring_t* o, void* arg);//2:生产者压入数据
 * void* linkstack_pop(linkstack_p o);//3:消费者弹出数据
 * linkstack_destory(linkstack_p o, linkstack_cbkfree cbkfree);//销毁对象
*/
#ifndef LLQ_LINKSTACK_H_
#define LLQ_LINKSTACK_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "listhead.h"

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

/** 条件等待与互斥锁实现的(数据资源等待链) */
typedef struct linkstack {
	pthread_cond_t	ready;//条件变量
	pthread_mutex_t	lock;//互斥锁
    listhead_t chian;//数据链
	uint32_t count;//挂勾上记录条数
} linkstack_t, *linkstack_p;
typedef struct liststact_entry{
    listhead_t chian;//数据链
    void* data;
} liststact_entry_t, liststact_entry_p;

typedef void (*linkstack_cbkfree)(void*);//链上资源释放函数

/** 初始化(数据资源等待链) */
static INLINE void linkstack_initialize(linkstack_p o) {
    memset(o, 0, sizeof(linkstack_t));
    listhead_p chian = &o->chian;
    INIT_LIST_HEAD(chian);//初始化链表
    pthread_mutex_init(&o->lock, NULL);
    pthread_cond_init(&o->ready, NULL);
}

/** 销毁(数据资源等待链) */
static INLINE void linkstack_destory(linkstack_p o, linkstack_cbkfree cbkfree){
    liststact_entry_p pos, next, listhead = &o->chian;
    listhead_for_each_entry_safe(pos, next, listhead, chian) {
        listhead_del(pos);
        if(cbkfree) {
            void* ptr = listhead_entry(pos, liststact_entry_p, data);
            if(ptr) { cbkfree(ptr); }
        }
        free(pos);
    }
    pthread_mutex_destroy(&o->lock);
    pthread_cond_destroy(&o->ready);
}

/** 从(数据资源等待链)弹出一个数据 */
static INLINE void* linkstack_pop(linkstack_p o) {
    void *ptr = NULL;
    pthread_mutex_lock(&o->lock);
    listhead_p hdr = &o->chian, first;//获得链表头
    while(listhead_empty(hdr)) { //线程阻塞,等待条件成立
        pthread_cond_wait(&(o->ready), &(o->lock));
    }
    first = hdr->next;
    ptr = listhead_entry(first, liststact_entry_p, data);
    listhead_del(first);//从链表中删除
    free(first);//回收资源
    o->count--;//将挂勾记数减一
    pthread_mutex_unlock(&o->lock);
}

/** 向(数据资源等待链)压入一个数据 */
static INLINE void linkstack_push(threadring_t* o, void* arg) {
    liststact_entry_p item = NULL;
    pthread_mutex_lock(&o->lock);
    if((item = calloc(sizeof(liststact_entry_t), 1))) {
        listhead_p hdr = &item->chian;//获得链表头
        INIT_LIST_HEAD(hdr);//初始化链表头
        item->data = arg;
        listhead_add_tail(&o->chian, hdr);//将记录添加到记录尾部
        o->count++;//将挂勾记数减一
    }
    pthread_cond_signal(&(o->ready));//通知等待带列
    pthread_mutex_unlock(&o->lock);
}

#endif