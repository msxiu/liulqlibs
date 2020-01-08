#ifndef LLQ_SIGNAL_H_
#define LLQ_SIGNAL_H_

#include <signal.h>


typedef void (*llqsignal_t)(int);
typedef void  (*llqsigalaction_t)(int, siginfo_t *, void *);

/**注册信号处理函数，成功反回1,失败返回0*/
static inline int llqsignal_hook(int signum, llqsignal_t handler) {
	void* ret = signal(signum, SIG_IGN);
	if((NULL != ret) && (handler)) {
		ret = signal(signum, handler);
	}
	return (NULL != ret);
}

/** 注册信号处理函数，成功返回0,失败返回-1 */
static inline int llqsigaction_hook(int signum, llqsigalaction_t handler){
    struct sigaction sa;//避免产生僵尸进程 注册子子进程退出忽略信号
    sigemptyset(&sa.sa_mask);
    if(handler) {
        sa.sa_flags = SA_SIGINFO;//使用了SA_SIGINFO标志时，使用该信号处理程序
    	sa.sa_sigaction = handler;
    } else {
    	sa.sa_flags = 0;
		sa.sa_handler = SIG_IGN;
    }
    return sigaction(signum, &sa, NULL);
}


#endif //LLQ_SIGNAL_H_
