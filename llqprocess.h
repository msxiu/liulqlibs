#ifndef LLQ_PROCESS_H_
#define LLQ_PROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#include "llqsignal.h"


typedef int (*llqprochandle_t)(void*);
/**创建子进程,在当前的进程环境内执行函数中的操作*/
static inline int llqprocess_execute(llqprochandle_t handler, void* arg) {
    int   ret = -1;
    if((ret = llqsigaction_hook(SIGCHLD, NULL)) >= 0 ) {
		if ((ret = fork()) == 0) {
			exit(handler(arg));
		}
    }
    return ret;
}

/**创建demon进程，跟据返回值判断是子进程还是父进程
* return 0:表示父进程
* return >0:表示子进程
* return -1:登记信号处理出现异常
* return -2:设置为会话组长时出现异常
*/
static inline pid_t llqprocess_daemon() {
    int   i, maxfd;
    pid_t pid = 0;
    if(llqsigaction_hook(SIGCHLD, NULL) < 0) {
    	return -1;
    }
    if ( (pid = fork()) != 0) {//pid != 0 父进程中,返回,exit(0);
        return pid;
    }
    if (setsid() < 0) {//子进程，后台继续执行,第一子进程成为新的会话组长和进程组长
        return -2;
    }
	chdir("/");//返回根目录
    umask(0);
    maxfd = getdtablesize();
    for (i = 0; i < maxfd; i++) {
        close(i);
    }
    int fd0 = open("/dev/null", O_RDWR);
    dup2(fd0, 1);
    dup2(fd0, 2);
    return pid;
}
static inline void llqdeamon_process() {
    if (setsid() < 0) { return; }
	chdir("/");//返回根目录
    umask(0);
    int fd0 = open("/dev/null", O_RDWR);
    dup2(fd0, 1);
    dup2(fd0, 2);
}

/**创建daemon进程，执行命令*/
static inline pid_t llqprocess_daemon_execute(char * path, char* const argv[]) {
    pid_t pid = llqprocess_daemon();
    if (pid == 0) {//在子进程中执行
        if (execv(path, argv) < 0) {
            printf("execl error: %s!\n", strerror(errno));
        }
        exit(-1);
    }
    return pid;

}



#endif //LLQ_PROCESS_H_

