/*
 * llqtrack.h
 *  Created on: 2017-7-6
 *      Author: Administrator
 *      用于应用程序的调试跟踪
static trackstream_t apptrack = { 	NULL, TRACKSTREAM_HEAD_TIME|TRACKSTREAM_HEAD_FILE, NULL, NULL };
static ptracknode_t track_process = {TRACKSTREAM_RANK_NONE, "process", "track process invoke!", NULL};
trackstream_attachment(&apptrack, &track_process, NULL);//调试链挂载
trackstream_close(&apptrack);//关闭调试

#define PROCESS_DEBUG(format, ...)  TRACESTREAM_DIAGNOSE(ptrack_process, TRACKSTREAM_RANK_DEBUG, format, __VA_ARGS__)
 */

#ifndef LILBTRACK_H_
#define LILBTRACK_H_

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctype.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif


#ifndef DIAGNOSE_NOTIFY
#define DIAGNOSE_NOTIFY		1
#endif
#ifndef DIAGNOSE_HEADMASK
#define DIAGNOSE_HEADMASK	7
#endif

/**快速链表头部添加节点,快速链表是指含有head与tail两个指针的链表 *@parameter {obj:链表对象, head:链表的头指针, tail:链表的尾指针, member:保存下一对象的成员名, value:添加的成员对象} */
#define CHAINFAST_ADDNODE(obj,head,tail,member,value)		do {\
	(value)->member = NULL;\
	if(!(obj)->head || !(obj)->tail) {\
		(obj)->head = (obj)->tail = (value);\
	} else {\
		((obj)->tail)->member=(value);((obj)->tail)=value;\
	}\
}while(0)
/**链表对象的遍厉 *@parameter {o:链表对象, n:下一对象指针, m:保存下一对象的成员名} * */
#define CHAINLIST_FOREACH(o, p, m)		for((p)=(o);(p);(p)=(p)->m)
/**链表对象的安全遍厉 *@parameter {o:链表对象, n:下一对象指针, m:保存下一对象的成员名} * */
#define CHAINLIST_EACHSAFE(o,p,n,m)		for((p)=(o),(n)=((o)?(o)->m:NULL);(p);(p)=(n),(n)=((p)?(p)->m:NULL))


#define BITSPROMPT(bits, bit, format, args...)    do{\
    if((bits & bit)){ \
        fprintf(stdout, "%s:%d=>", __FILE__, __LINE__); \
        fprintf(stdout, format, ##args);\
		fprintf(stdout, "%s", "\r\n");\
		fflush(stdout);\
    }\
}while(0)



//枚举错误等级
typedef enum TRACKSTREAM_RANK {
	TRACKSTREAM_RANK_NONE	= 0,//不进行任何调试
	TRACKSTREAM_RANK_FATAL	= 1,//每个严重的错误事件将会导致应用程序的退出
	TRACKSTREAM_RANK_ERROR	= 2,//虽然发生错误事件，但仍然不影响系统的继续运行
	TRACKSTREAM_RANK_WARN	= 3,//出现潜在错误的情形
	TRACKSTREAM_RANK_INFO	= 4,//消息在粗粒度级别上突出强调应用程序的运行过程
	TRACKSTREAM_RANK_DEBUG	= 5,//指出细粒度信息事件
} trackrank_t;
typedef enum TRACKSTREAM_HEAD {
	TRACKSTREAM_HEAD_NAME = 1,//调试级别
	TRACKSTREAM_HEAD_TIME = 2,//时间
	TRACKSTREAM_HEAD_FILE = 4,//文件
} trackhead_t;

struct ptracknode_struct;
typedef struct ptracknode_struct ptracknode_t, * ptracknode_p;
struct trackstream_struct;
typedef struct trackstream_struct trackstream_t, * trackstream_p;

struct ptracknode_struct {
	enum TRACKSTREAM_RANK rank;//错误等级的位或码
	const char* name;//名称
	const char* note;//注示说明
	trackstream_p track;//根踪对象
	ptracknode_p next;//下一个节点对象地址
};
/** 跟踪调试 结构体,用于位或定义某项是否开启,开启后的输出文流及文件  */
struct trackstream_struct {
	FILE* stream;//输出的数据流
	int prefix;//输出前缀信息
	char addr[256];//输出 的文件
	ptracknode_p head;//单向链表头
	ptracknode_p last;//单向链表尾
};


/**开启o的i位跟踪
 *@parameter o:需要设置的ptracknode_t对象
 *@parameter v:指定TRACKSTREAM_RANK值
 */
#define TRACESTREAM_OPEN(o, v)	((o)->rank = (enum TRACKSTREAM_RANK)(v))



static INLINE const char* getenum_TRACKSTREAM_RANK(enum TRACKSTREAM_RANK v) {
	switch(v) {
	case TRACKSTREAM_RANK_DEBUG: return "debug";
	case TRACKSTREAM_RANK_INFO: return "info";
	case TRACKSTREAM_RANK_WARN: return "warn";
	case TRACKSTREAM_RANK_ERROR: return "error";
	case TRACKSTREAM_RANK_FATAL: return "fatal";
	default:return "0";
	}
}

/***打开调试输出流
 *@parameter o:trackstream_t跟踪流对象
 */
static INLINE FILE* trackstream_openstream(ptracknode_t* node) {
	if(node && node->track) {
		trackstream_t* o = node->track;
		if(!o->stream) {
			if(NULL == o->addr || !(*o->addr)) {
				o->stream = stdout;//文件地址为空时,直接赋值默认流
			} else {
				printf("open the trackstream for '%s'\n", o->addr);
				o->stream = fopen(o->addr, "a");
				if(NULL == o->stream) {
					o->stream = stdout;//打开文件失败时直接赋值默认流
				}
			}
		}
		return o->stream;
	}
	return stdout;
}
static INLINE void trackstream_close(trackstream_t* o) {
	if(o && o->stream && o->stream != stdout) {
		fclose(o->stream);//关闭打开文件对象
	}
}


/*** 时间格式化显示 */
static INLINE char* trackstream_timeformat(time_t t, char* buffer, int bln) {
	static const char* tmfmt = "%d %H:%M";
	if(t && buffer && bln) {
		int vln;
		char tmbuf[128];
		struct tm itm;
		memset(tmbuf, 0, sizeof(tmbuf));
#ifdef WIN32
		localtime_s(&itm, &t);
#else
		localtime_r(&t, &itm);
#endif
		strftime (tmbuf, sizeof(tmbuf), tmfmt, &itm);
		vln = strlen(tmbuf);
		if(bln > vln) {
			strcpy(buffer, tmbuf);
			return buffer;
		}
	}
	return NULL;
}
static INLINE void trackstream_head(FILE* fio, int prefix, enum TRACKSTREAM_RANK rank, const char* fn, int ln) {
	if(fio) {
		time_t t = time(NULL);
		char tbuf[64];
		switch(prefix) {//NAME,TIME,FILE
			case 1:fprintf(fio, "[%s] ", getenum_TRACKSTREAM_RANK(rank));break;//NAME
			case 2:fprintf(fio, "[%s] ", trackstream_timeformat(t, tbuf, sizeof(tbuf)));break;//TIME
			case 3:fprintf(fio, "[%s-%s] ", getenum_TRACKSTREAM_RANK(rank), trackstream_timeformat(t, tbuf, sizeof(tbuf)));break;//NAME+TIME
			case 4:fprintf(fio, "%s:%d ", fn, ln);break;//FILE
			case 5:fprintf(fio, "[%s]%s:%d ", getenum_TRACKSTREAM_RANK(rank), fn, ln);break;//NAME+FILE
			case 6:fprintf(fio, "[%s]%s:%d ", trackstream_timeformat(t, tbuf, sizeof(tbuf)), fn, ln);break;//TIME+FILE
			case 7:fprintf(fio, "[%s-%s]%s:%d ", getenum_TRACKSTREAM_RANK(rank), trackstream_timeformat(t, tbuf, sizeof(tbuf)), fn, ln);break;//NAME+TIME+FILE
		}
	}
}

#if defined(WIN32)	//windows平台下的宏定义
/**跟踪信息诊断
 *@parameter o:ptracknode_t对象指针
 *@parameter k:级别
 *@parameter format:格式字符串
 *@parameter args:参数
 */
#define TRACESTREAM_DIAGNOSE(o, k, format, ...) if((((o)->rank) >= (k))) {\
	FILE* fio = trackstream_openstream(o);\
	if(NULL != fio) {\
		trackstream_head(fio, o->track->prefix, k, __FILE__, __LINE__);\
		fprintf(fio, format, __VA_ARGS__);\
		fprintf(fio, "%s", "\r\n");\
		fflush(fio);\
	}\
}

#define TRACESTREAM_NOTE(o, format, args...) do{\
		FILE* fio = trackstream_openstream(o);\
		if(NULL != fio) {\
			trackstream_head(fio, o->track->prefix, 0, __FILE__, __LINE__); \
			fprintf(fio, format, __VA_ARGS__);\
			fprintf(fio, "%s", "\r\n");\
			fflush(fio);\
		}\
}while(0)
#define DIAGNOSE_PROMPT(k, format, ...)	if(((DIAGNOSE_NOTIFY) >= (k))) {\
	trackstream_head(stdout, DIAGNOSE_HEADMASK, k, __FILE__, __LINE__); \
	fprintf(stdout, format, __VA_ARGS__); fprintf(stdout, "%s", "\r\n"); fflush(stdout);\
}
#define DIAGNOSE_DEBUG(format, ...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_DEBUG, format, __VA_ARGS__)
#define DIAGNOSE_INFO(format, ...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_INFO, format, __VA_ARGS__)
#define DIAGNOSE_WARN(format, ...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_WARN, format, __VA_ARGS__)
#define DIAGNOSE_ERROR(format, ...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_ERROR, format, __VA_ARGS__)
#define DIAGNOSE_FATAL(format, ...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_FATAL, format, __VA_ARGS__)
#else
/**跟踪信息诊断
 *@parameter o:ptracknode_t对象指针
 *@parameter k:级别
 *@parameter format:格式字符串
 *@parameter args:参数
 */
#define TRACESTREAM_DIAGNOSE(o, k, format, args...) if((((o)->rank) >= (k))) {\
	FILE* fio = trackstream_openstream(o);\
	if(NULL != fio) {\
		trackstream_head(fio, o->track->prefix, k, __FILE__, __LINE__);\
		fprintf(fio, format, ##args);\
		fprintf(fio, "%s", "\r\n");\
		fflush(fio);\
	}\
}

#define TRACESTREAM_NOTE(o, format, args...) do{\
	FILE* fio = trackstream_openstream(o);\
	if(NULL != fio) {\
		trackstream_head(fio, o->track->prefix, 0, __FILE__, __LINE__);\
		fprintf(fio, format, ##args);\
		fprintf(fio, "%s", "\r\n");\
		fflush(fio);\
	}\
}while(0)
/**向控制台输出诊断提示*/
#define DIAGNOSE_PROMPT(k, format, args...)	if(((DIAGNOSE_NOTIFY) >= (k))) {\
	trackstream_head(stdout, 7, k, __FILE__, __LINE__); \
	fprintf(stdout, format, ##args); fprintf(stdout, "%s", "\r\n"); fflush(stdout);\
}
#define DIAGNOSE_DEBUG(format, args...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_DEBUG, format, ##args)
#define DIAGNOSE_INFO(format, args...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_INFO, format, ##args)
#define DIAGNOSE_WARN(format, args...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_WARN, format, ##args)
#define DIAGNOSE_ERROR(format, args...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_ERROR, format, ##args)
#define DIAGNOSE_FATAL(format, args...) DIAGNOSE_PROMPT(TRACKSTREAM_RANK_FATAL, format, ##args)

#endif

/**调试执行相关程序
 *@parameter o:ptracknode_t对象指针
 *@parameter k:级别
 */
#define TRACESTREAM_ALLOW(o, k)	(((o)->rank) >= (k))
/**调试执行相关程序
 *@parameter o:ptracknode_t对象指针
 *@parameter k:级别
 *@parameter execute:执行函数
 */
#define TRACESTREAM_CALLBACK(o, k, execute) if(TRACESTREAM_ALLOW(o,k)) {execute;}






#define SECTION_SYMBOL "----- %s ----- %s -----------------------------------"
/**跟踪流信息显示
 *@parameter o:trackstream_t跟踪流对象
 *@parameter stream:显示信息流
 */
static INLINE void trackstream_view(trackstream_t* o, FILE* stream) {
	int i = 0;
	fprintf(stream, SECTION_SYMBOL, "track stream", "finish");
	fprintf(stream, "%p=>{stream:(%p),addr:'%s'}\n", o, o->stream, o->addr);
	if(o && o->head) {
		ptracknode_p head = o->head, p, n;
		CHAINLIST_EACHSAFE(head, p, n, next) {
			fprintf(stream, "(%u:%p):'%s'=>(%u:%s)", i, p, p->name, p->rank, getenum_TRACKSTREAM_RANK(p->rank));
			fprintf(stream, ", %s\n", p->note);
		}
	}
	fprintf(stream, SECTION_SYMBOL, "track stream", "finish");
}
/**给测试流附加测试选项
 *@parameter o:trackstream_t跟踪流对象
 *@return 返回设置的参数个数
 */
static INLINE int trackstream_attachment(trackstream_t* o,...) {
	va_list argp;
	int cnt = 0;
	ptracknode_t* par;//, *nargp指向传入的第一个可选参数，msg是最后一个确定的参数
	if(o) {
		va_start(argp, o);//指向可变参数
		while (1) {
			par = va_arg(argp, ptracknode_t*);//读取可变参数值
			if (NULL == par) break;
			cnt++;
			par->track = o;
			CHAINFAST_ADDNODE(o, head, last, next, par);
		}
		va_end(argp);//将argp置为NULL
	}
	return cnt;
}

static INLINE int trackstream_render(trackstream_t* o, enum TRACKSTREAM_RANK rank) {
	int rcnt = 0;
	if(o && o->head) {
		ptracknode_t *p = o->head, *n;
		CHAINLIST_FOREACH(p, n, next) {
			n->rank = rank;
			rcnt++;
		}
	}
	return rcnt;
}
/**设置调试选项值
 *@parameter o:trackstream_t跟踪流对象
 *@parameter item:选项字符串{utils=debug}
 */
static INLINE int trackstream_setnode(trackstream_t* o, const char* name, const char* value) {
	int biv=-1;
	if(o && o->head && name && value) {
		ptracknode_t *p = o->head, *n;
		CHAINLIST_FOREACH(p, n, next) {
			if(0 == strcasecmp(name, n->name)) {
				if(isdigit(*value)) {//第一个字符为数字
					biv = atoi(value);
				} else if(0 == strcasecmp("debug", value)|| 0 == strcasecmp("on", value)) {
					biv = TRACKSTREAM_RANK_DEBUG;
				} else if(0 == strcasecmp("info", value)) {
					biv=TRACKSTREAM_RANK_INFO;
				} else if(0 == strcasecmp("warn", value)) {
					biv=TRACKSTREAM_RANK_WARN;
				} else if(0 == strcasecmp("error", value)) {
					biv=TRACKSTREAM_RANK_ERROR;
				} else if(0 == strcasecmp("fatal", value)) {
					biv=TRACKSTREAM_RANK_FATAL;
				} else if(0 == strcasecmp("off", value) || 0 == strcasecmp("none", value)) {
					biv=TRACKSTREAM_RANK_NONE;
				}
				n->rank = (enum TRACKSTREAM_RANK)biv;
				return 1;
			}
		}
	}
	return 0;
}

// static INLINE int trackstream_setkvitem(
// 	trackstream_t* o,	//trackstream_t跟踪流对象
// 	const char* item,	//选项字符串{utils=debug}
// 	keyvalarea_p a		//键值位置对象
// ){
// 	char name[128], value[8];
// 	if(o && item && a) {
// 		sprintf(name, "%.*s", (a->key.finish - a->key.start), (item + a->key.start));
// 		sprintf(value, "%.*s", (a->val.finish - a->val.start), (item + a->val.start));
// 		return trackstream_setnode(o, name, value);
// 	}
// 	return 0;
// }
/**设置调试选项值,@return 成功返回1,失败返回0*/
// static INLINE int trackstream_setitem(
// 	trackstream_t* o,	//trackstream_t跟踪流对象
// 	const char* item	//选项字符串{utils=debug}
// ) {
// 	keyvalarea_t a;
// 	if(strchrbuffer_keyvalof(item, '=', &a, 0x33)) {
// 		return trackstream_setkvitem(o, item, &a);
// 	}
// 	return 0;
// }
// /**设置调试选项数组的值, @return返回设置成功个数*/
// static INLINE int trackstream_setitems(
// 	trackstream_t* o,	//trackstream_t跟踪流对象
// 	const char* str		//'name=debug,name1=info,name2=debug'的选项
// ) {
// 	int rcnt = 0, ret = 0, i;
// 	keyvalarea_p areas = NULL;
// 	if((rcnt = strchrbuffer_kvsplit(str, '=', ',', &areas, 0x33))) {
// 		for(i=0; i<rcnt; i++) {
// 			ret += trackstream_setkvitem(o, str, (areas + i));
// 		}
// 		free(areas);
// 	}
// 	return ret;
// }

#endif /* LILBTRACK_H_ */
