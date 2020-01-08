#ifndef LLQ_PARAMETERS_H_
#define LLQ_PARAMETERS_H_
#include <string.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

/**命令参数分解,将一个命令字符中,解析成一个数组, return:返回解析到的有效参数个数  */
static INLINE int parameters_parse(char *s, int argvsz, char *argv[]) {
	int argc = 0;
	if(s && argv && argvsz > 0) {
		while (argc < argvsz - 1) {
			while ((*s == ' ') || (*s == '\t') || (*s == '\r') || (*s == '\n')) ++s;
			if (*s == '\0')	break;
			argv[argc++] = s;
			while (*s && (*s != ' ') && (*s != '\t') &&  (*s != '\r') && (*s != '\n')) ++s;
			if (*s == '\0') break;
			*s++ = '\0';
		}
		argv[argc] = NULL;
	}
	return argc;
}

/** 查找参数值 */
static INLINE int parameters_find(int argc, const char* argv[], const char* key, const char** value) {
	int i=0;
	*value = NULL;
    for(i=0; i<argc; i++) {
        if(0 == strcasecmp(key, argv[i])) {
            if(NULL != value && argc > i+1) {
                const char* v = argv[i+1];
                if('-' != *v) { *value = v; }//赋值数据
            }
            return 1;
        }
    }
    return 0;
}
/** 查找参数出现的下标位置 */
static INLINE int parameters_indexof(int argc, const char* argv[], const char* key, int i) {
	*value = NULL;
    for(; i<argc; i++) {
        if(0 == strcasecmp(key, argv[i])) {
			return i+1;//跳过当前参数值到下一个参数位置
        }
    }
    return 0;
}

#endif 
