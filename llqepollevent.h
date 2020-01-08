#ifndef LLQ_EPOLL_EVENT_H
#define LLQ_EPOLL_EVENT_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/types.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

static INLINE int epollevent_remove(int epoll, int sock) {
    return (-1 != epoll_ctl(epoll, EPOLL_CTL_DEL, sock, NULL));
}
static INLINE int epollevent_read(int epoll, int sock, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    if(NULL == ptr) {
        ev.data.fd = sock;
    } else {
        ev.data.ptr = ptr;
    }
    return (-1 != epoll_ctl(epoll, EPOLL_CTL_ADD, sock, &ev));
}
static INLINE int epollevent_write(int epoll, int sock, void* ptr) {
    struct epoll_event ev;
    ev.events = EPOLLOUT | EPOLLET;
    if(NULL == ptr) {
        ev.data.fd = sock;
    } else {
        ev.data.ptr = ptr;
    }
    return (-1 != epoll_ctl(epoll, EPOLL_CTL_ADD, sock, &ev));
}


#endif