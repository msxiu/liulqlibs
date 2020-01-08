#ifndef LLQ_FIXED_ARRAY_PTR_H_
#define LLQ_FIXED_ARRAY_PTR_H_
#include <stdlib.h>
#include <stdint.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

typedef int (*fixedarrayptr_compare)(const void* a, const void * b);

static INLINE int fixedarrayptr_append(void* array[], size_t maxsize, size_t* idx, void* data) {
    if(array && idx && (*idx) < (maxsize - 1)) {
        array[*idx] = data;
        (*idx)++;
        return 1;
    }
    return 0;
}
static INLINE int fixedarrayptr_indexof(void* array[], size_t count, const void* data, fixedarrayptr_compare compare) {
    int idx = 0, mxidx = (int)count;
    for(idx = 0; idx < mxidx; idx++) {
        if(0 == compare(data, array[idx])) return idx;
    }
    return -1;
}

#endif