#ifndef LLQ_SIZEPTR_H_
#define LLQ_SIZEPTR_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

static INLINE int fixedbuffer_indexof(const void* buf, size_t bln, size_t p, unsigned char c) {
	size_t i=0;
	unsigned char ch;
	if(buf && p < bln) {
		while(i < bln) {
			ch = *(((unsigned char*)buf) + (p + i));
			i++;
			if(ch == c) return i;
		}
	}
	return -1;
}

/**读取指定长度的数据到缓存区中*/
static INLINE size_t fixedbuffer_read(const void* buf, size_t bln, size_t *p, void* v, size_t l) {
	size_t i = 0;
	if(buf && p && (*p)<bln && ((*p)+l)<=bln){
		memcpy(v, (((const unsigned char*)buf) + (*p)), l);
        i = l;
		(*p) += i;
	}
	return i;
}
static INLINE size_t fixedbuffer_readint8(const void* buf, size_t bln, size_t *p, int8_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(int8_t)); }
static INLINE size_t fixedbuffer_readint16(const void* buf, size_t bln, size_t *p, int16_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(int16_t)); }
static INLINE size_t fixedbuffer_readint32(const void* buf, size_t bln, size_t *p, int32_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(int32_t)); }
static INLINE size_t fixedbuffer_readint64(const void* buf, size_t bln, size_t *p, int64_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(int64_t)); }

static INLINE size_t fixedbuffer_readuint8(const void* buf, size_t bln, size_t *p, uint8_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(uint8_t)); }
static INLINE size_t fixedbuffer_readuint16(const void* buf, size_t bln, size_t *p, uint16_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(uint16_t)); }
static INLINE size_t fixedbuffer_readuint32(const void* buf, size_t bln, size_t *p, uint32_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(uint32_t)); }
static INLINE size_t fixedbuffer_readuint64(const void* buf, size_t bln, size_t *p, uint64_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(uint64_t)); }

static INLINE size_t fixedbuffer_readsizet(const void* buf, size_t bln, size_t *p, size_t* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(size_t)); }
static INLINE size_t fixedbuffer_readfloat(const void* buf, size_t bln, size_t *p, float* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(float)); }
static INLINE size_t fixedbuffer_readdouble(const void* buf, size_t bln, size_t *p, double* v){ return fixedbuffer_read(buf, bln,  p, v, sizeof(double)); }


/**从指定位置读取一个字符串*/
static INLINE const char* fixedbuffer_stringof(const void* buf, size_t bln, size_t *p) {
	int i=0;
	const char* s = NULL;
	if(buf && bln && p && (-1 != (i=fixedbuffer_indexof(buf, bln, *p, 0)))) {
		s = (((const char*)buf) + (*p));
		*p+=i;
	}
	return s;
}
static INLINE int fixedbuffer_keyvalueof(const void* buf, size_t bln, size_t *p, const char** key, const char** value) {
	int i=0;
	if(buf && bln && p && (bln > (*p))) {
        if(-1 != (i = fixedbuffer_indexof(buf, bln, *p, 0))) {
            *key = (((const char*)buf)+(*p));
            *p += i;
            if(-1 != (i = fixedbuffer_indexof(buf, bln, *p, 0))) {
                *value = (((const char*)buf)+(*p));
                *p += i;
                return 1;
            }
        }
	}
	return 0;

}

/**从指定位置读取到c出现的位置,包括c到v中,v的最大长度是l*/
static INLINE int fixedbuffer_readto(const void* buf, size_t bln, size_t *p, unsigned char c, void* v, size_t l) {
	int i = 0;
	size_t p2;
	if(buf && bln && p && v && l && (-1 != (i=fixedbuffer_indexof(buf, bln, *p, c)))) {
		p2 = (size_t)i;
		memcpy(v, (((unsigned char*)buf)+(*p)), (p2 <= l ? p2 : l));
		(*p) += i;
	}
	return i;
}
/**从指定位置读取到c出现的位置,包括c到v中,v的最大长度是l*/
static INLINE int fixedbuffer_readline(const void* buf, size_t bln, size_t *p, char* v, size_t l) {
	int i = 0, ret = 0;
	if(buf && bln && p && v && l && (-1 != (i=fixedbuffer_indexof(buf, bln, *p, '\n')))) {
		if(i > 1) {
			ret = i -1;
			memcpy(v,  (((unsigned char*)buf)+(*p)),  (((size_t)ret) <= l ? ((size_t)ret) : l));
			if('\r' == *(v + ret - 1)) { ret--; *(v+ret)=0; }
		}
		(*p) += i;
	}
	return ret;
}
static INLINE size_t fixedbuffer_readbuffer(const void* buf, size_t bln, size_t *p, void* v, uint32_t *vl) {
	uint32_t r = 0;
	if(sizeof(uint32_t) == fixedbuffer_readuint32(buf, bln, p, &r)) {
		if(r == fixedbuffer_read(buf, bln,  p, v, (size_t)r)) {
			if(vl) { *vl = (r + sizeof(uint32_t)); }
			return (r + sizeof(uint32_t));
		}
	}
	return 0;
}

#endif