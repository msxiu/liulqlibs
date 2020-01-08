#ifndef LLQ_CHARSET_DETECT_H_
#define LLQ_CHARSET_DETECT_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

typedef enum UNSAFECHAR_TYPE {
	UNSAFECHAR_NONE			= 0,//安全字符
	UNSAFECHAR_JSONMAP		= 1,//JSON不安全,需要斜杠转换
	UNSAFECHAR_JSONCODE		= 2,//JSON不安全,需要编码处理
	UNSAFECHAR_URL			= 4,//URL不安全
	UNSAFECHAR_DISK			= 8,//磁盘路径不安全
} unsafechar_t;

/**字符安全检测*/
static unsafechar_t charectersafedetect(unsigned char c) {
	switch(c){//单个管理字符
	case '\a'://Sound alert
	case '\b'://退格
	case '\f'://Form feed
	case '\n'://换行
	case '\r'://回车
	case '\t'://水平制表符
	case '\v'://垂直制表符
		return (UNSAFECHAR_JSONMAP | UNSAFECHAR_URL);
	case 32:return UNSAFECHAR_URL;//空格
	case 33/*'!'*/: return UNSAFECHAR_DISK;
	case 34/*'"'*/:return (UNSAFECHAR_JSONMAP | UNSAFECHAR_URL | UNSAFECHAR_DISK);
	//case 35/*'#'*/:
	//case 36/*'$'*/:
	case 37/*'%'*/:
	case 38/*'&'*/: return (UNSAFECHAR_DISK|UNSAFECHAR_URL);
	case 39/*'\''*/:return (UNSAFECHAR_JSONMAP | UNSAFECHAR_URL | UNSAFECHAR_DISK);
	//case 40/*'('*/:
	//case 41/*')'*/:
	case 42/*'*'*/:	return UNSAFECHAR_DISK;
	case 43/*'+'*/: return UNSAFECHAR_URL;
	//case 44/*','*/:
	//case 45/*'-'*/:
	//case 46/*'.'*/:
	case 47/*'/'*/:return (UNSAFECHAR_JSONMAP | UNSAFECHAR_URL);
	//case 58/*':'*/:
	//case 59/*';'*/:
	case 60/*'<'*/: return (UNSAFECHAR_JSONCODE | UNSAFECHAR_URL | UNSAFECHAR_DISK);
	case 61/*'='*/: return UNSAFECHAR_URL;
	case 62/*'>'*/: return (UNSAFECHAR_JSONCODE | UNSAFECHAR_URL | UNSAFECHAR_DISK);
	case 63/*'?'*/: return (UNSAFECHAR_URL | UNSAFECHAR_DISK);
	//case 64/*'@'*/:
	//case 91/*'['*/:
	case 92/*'\'*/: return (UNSAFECHAR_JSONMAP | UNSAFECHAR_URL);
	//case 93/*']'*/:
	case 94/*'^'*/: return UNSAFECHAR_DISK;
	//case 95/*'_'*/:
	case 96/*'`'*/: return (UNSAFECHAR_JSONCODE | UNSAFECHAR_URL);
	case 123/*'{'*/: return UNSAFECHAR_URL;
	case 124/*'|'*/: return UNSAFECHAR_DISK;
	case 125/*'}'*/: return UNSAFECHAR_URL;
	case 126/*'~'*/: return UNSAFECHAR_DISK;
	case 127: return (UNSAFECHAR_JSONCODE | UNSAFECHAR_URL | UNSAFECHAR_DISK);
	default:
		//if(c>33 && c<127) { return UNSAFECHAR_NONE; }//可见字符部分
		return UNSAFECHAR_NONE;//小于33或大于127部分需使JSON编码
	}
	//if((c>=48 && c <= 57) || (c>=65 && c<=90) || (c>=97 && c<=122)) {//合法字符
	//	return UNSAFECHAR_NONE;
	//}
}


#endif
