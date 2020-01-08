#ifndef LLQ_SOCKET_H_
#define LLQ_SOCKET_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "llqvarymem.h"

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

typedef int sock_t;//套接字标识,未链接时或链接出错时,使用-1表示

/**异常识别与处理, @return {1:continue,0:return(返回失败)} */
static INLINE int socket_exception() {
	switch(errno) {
	case EAGAIN://Try again 重试
	case EINTR://Interrupted system call*中断的系统调用
	case EBUSY://Device or resource busy*设备或资源忙
		return 1;
	}
	return 0;
}

/**获得流套接字 *@return 成功返回套按字,失败返回-1 */
static INLINE sock_t socket_getstream() {
    static const int families[] = { AF_INET, AF_IPX, AF_AX25, AF_APPLETALK };
    sock_t	sock;
	size_t i;
    for(i = 0; i < (sizeof(families)/sizeof(int)); ++i) {
        if((sock = socket(families[i], SOCK_STREAM, 0)) > 0) return sock;
    }
    return -1;
}
/**获得数据报套接字 *@return 成功返回套按字,失败返回-1 */
static INLINE sock_t socket_getdgram(void) {
    static const int families[] = { AF_INET, AF_IPX, AF_AX25, AF_APPLETALK };
    sock_t	sock;
	size_t i;
    for(i = 0; i < (sizeof(families)/sizeof(int)); ++i) {
        if((sock = socket(families[i], SOCK_DGRAM, 0)) >=0) return sock;
    }
    return -1;
}
/**设置套接字为非阻赛方式, @return 成功反回1,失败返回0 */
static INLINE int socket_nonblock(sock_t fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	return ((-1 != fcntl(fd, F_SETFL, flags | O_NONBLOCK)) ? 1 : 0);
}
/** 在sockfd_one绑定bind之前，设置其端口复用 */
static INLINE int socket_reuse(sock_t fd) {
	int opt = 1;
	return (0 == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,  (const void *)&opt, sizeof(opt)));
}

/**套接字链接检测, @return 成功返回1,失败返回0 */
static INLINE int socket_conncheck(sock_t sockfd) {
     int ret = 0;
     socklen_t len = sizeof(ret);
     if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &ret, &len) == -1) {
         return 0;
     }
     return (ret == 0 ? 1 : 0);
 }
/**设置套接字接收超时, @return 成功返回1,失败返回0 */
static INLINE int socket_timeout_recv(sock_t fd, int sec) {
	if(fd>0 &&sec > 0) {//设置发送超时
		struct timeval ti = {sec, 0};
		return (0 == setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &ti, sizeof(struct timeval)));
	}
	return 0;
}
/**设置套接字发送超时, @return 成功返回1,失败返回0 */
static INLINE int socket_timeout_send(sock_t fd, int sec) {
	if(fd>0 &&sec > 0) {//设置发送超时
		struct timeval ti = {sec, 0};
		return (0 == setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &ti, sizeof(struct timeval)));
	}
	return 0;
}

/**建立套接字连接, @return 连接成功返回1,否则返回0 */
static INLINE int socket_connect(sock_t fd, const char* ip, unsigned short port) {
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;//inet_pton();
	sockaddr.sin_addr.s_addr = inet_addr(ip);//ip
	sockaddr.sin_port = htons(port);//port
	return (connect(fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) != -1);
}
/**创建套接字,并绑定IP与端口*/
static INLINE sock_t socket_openbind(sock_t fd, const char* ip, int port) {
	struct sockaddr_in sockaddr;
	memset(&sockaddr, 0,sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;//inet_pton(AF_INET, ip, &sockaddr.sin_addr);
	if(ip && *ip) {
		sockaddr.sin_addr.s_addr = inet_addr(ip);//ip
	} else {
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);//任何地址
	}
	sockaddr.sin_port = htons(port);
	return (-1 != bind(fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr)));
}

/**读取阻塞socket中的已接收到缓存区的数据,可能不是一个完整的协议,
 *  @return 返回读取到的数据,malloc出的内存,需回收,失败返回NULL */
static INLINE int socket_receive(sock_t fd, sizeptr_p data, uint32_t* maxsize) {
	int retval = 0, reals = 0;
	char hv[1024];
	if((fd > 0) && data) {
		while(1) {
			if(-1 == (reals = recv(fd, hv, sizeof(hv), 0))) {
                if (errno == EAGAIN) continue;//非阻塞模式下调用了阻塞操作
				return -1;//读取失败
			}
			if(0 == reals) break;//没有读取到标志,表示失败
			retval += reals;
			varysizeptr_write(data, maxsize, hv, reals);//向缓存写入数据
			if(reals < (int)sizeof(hv)) break;//未读满数据，表示缓存区数据已读完
		}
	}
	return retval;
}

/**从阻塞套接字中读取数据, @return 读取成功,返回>0,否则返回-1,可通过errno获得错误信息  */
static INLINE int socket_read(sock_t fd, void* buffer, size_t len) {
	size_t nextlen = len, reals = 0;
	unsigned char* dstart = (unsigned char*)buffer;
	while(nextlen > 0) {
		if((size_t)(-1) == (reals = recv(fd, dstart, nextlen, 0))) {
			if(socket_exception()) continue;//系统资源等问题,需要再次读取
			return -1;//读取失败
		}
		if(0 == reals) return -1;//阻塞同读取，返回0表示缓存区中没有数据，再次读取
        nextlen -= reals;
        dstart += reals;
	}
	return (nextlen == 0 ? (int)len : -1);
}

/**向阻塞套接字中写入数据, @return 写入成功,返回>0,否则返回-1,可通过errno获得错误信息 */
static INLINE int socket_write(sock_t fd, const void* buffer, size_t len) {
	size_t nextlen = len, reals = 0;
	unsigned char* dstart = (unsigned char*)buffer;
	while(nextlen > 0) {
		if((reals = send(fd, dstart, nextlen, 0)) <= 0){
			if(socket_exception()) continue;//系统资源等问题,需要再次读取
			return -1;//读取失败
		} 
		if(0 == reals) continue;//返回0表示缓存区已满，再次调用写
        nextlen -= reals;
        dstart += reals;
	}
	return (nextlen == 0 ? (int)len : -1);
}



/**从非阻赛套接字读数据，当缓存区中数据读取完时返回*/
static INLINE size_t socket_nonblock_read(int sock, sizeptr_p ptr, size_t* msize) {
    size_t nread;
    char buffer[1024];
    if(sock > 0 && ptr) {
        memset(ptr, 0, sizeof(sizeptr_t));
		while (1) {
			if ((nread = read(sock, buffer, sizeof(buffer))) < 0) {
				if (errno == EAGAIN) continue;//非阻塞模式下调用了阻塞操作
				return -1; //错误，返回-1
			} else if (nread == 0) {
				break; //已完成读取,退出循环
			}
            varysizeptr_write(ptr, (uint32_t*)msize, buffer, (uint32_t)nread);//写入缓存区
		}
    }
    return ptr->size; //返回读取到的数据字节数
}
/**向非阻赛套接字写数据，当缓存区写满时返回*/
static INLINE size_t socket_nonblock_write(int sock, sizeptr_p ptr, size_t* offset) {
    size_t nwritten, wln = 0;
    if(sock > 0 && ptr && offset && (*offset) < ptr->size) {
		while(ptr->size > (*offset)) {
			if ( (nwritten = write(sock, ptr->ptr, (ptr->size - (*offset)))) < 0)  {
				if (errno == EAGAIN) return 0;//调用了阻塞，表示缓存区已满，等待再次调用
				return -2;//错误，返回-1
			}
			if (nwritten == 0) return -1;//错误，返回-1 对方套接字关闭
            wln += nwritten;
		}
		(*offset) += wln;
    }
    return wln;
}


#endif