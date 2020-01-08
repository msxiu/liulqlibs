#ifndef LLQ_REFLECT_H_
#define LLQ_REFLECT_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32)
#define INLINE __inline
#else
#define INLINE inline
#endif


//计算成员偏移位置
#define PTRMEMBER_OFFSET(type, member)		((size_t)(&((type *)0)->member))
/**获得父对象首指针, @parameter{prt:当前对象指针, type:结构体类型, member:成员名} */
#define PRTMEMBER_PARENT(ptr, type, member) 	((type *)((char *)(ptr)-PTRMEMBER_OFFSET(type, member) ))
//计算成员空间大小
#define PTRMEMBER_SIZEOF(type, member)		(sizeof(((type *)0)->member))

typedef enum REFLECT_TYPE {//反射类型枚举
	REFLECT_NONE		= 0x00,//未知
	REFLECT_INT			= 0x01,//符号整型
	REFLECT_UINT		= 0x02,//无符号整型
	REFLECT_TIME		= 0x03,//时间(time_t)类型
	REFLECT_FLOAT		= 0x04,//浮点
	REFLECT_BINBUF		= 0x05,//固定大小缓存区
	REFLECT_STRBUF		= 0x06,//字符串缓存区
	REFLECT_STRING		= 0x07,//字符串指针
	REFLECT_OBJBUF		= 0x08,//对象缓存区
	REFLECT_OBJECT		= 0x09,//对象指针
} reflectenum_t;

struct reflect;
typedef struct reflect reflect_t, * reflect_p;
typedef reflect_p (*getentityreflect_t)();

/** 反射引用结构体，用于存储反射信息 */
struct reflect {
	uint32_t	entire;//被反射结构体总体长度
	const char * fldname;//字段名称
	reflectenum_t type;//数据类型
	uint16_t offset;//相对于结构体偏移
	uint16_t size;//成员在结构体中占用字节长度
	getentityreflect_t getreflect;
};
/** 实例为空，一般用在数组尾部 */
#define REFLECT_EMPTY											{ 0, NULL, REFLECT_NONE, 0, 0, NULL}
/** 简单初始化 */
#define REFLECT_INIT(reftype, objtype, member)			\
	{ sizeof(objtype), #member, reftype, PTRMEMBER_OFFSET(objtype, member), PTRMEMBER_SIZEOF(objtype, member), NULL}
/** 回调初始化，对于复杂对象，可能本就是一个结构体，可以使用回调返回该成员的反射引用数据 */
#define REFLECT_CALLBACK(reftype, objtype, member, handle)				\
	{ sizeof(objtype), #member, reftype, PTRMEMBER_OFFSET(objtype, member), PTRMEMBER_SIZEOF(objtype, member), handle}
/** 命名初始化，在反射中使用给定的名称初始化对象 */
#define REFLECT_NAMED(name, reftype, objtype, member)			\
	{ sizeof(objtype), name, reftype, PTRMEMBER_OFFSET(objtype, member), PTRMEMBER_SIZEOF(objtype, member), NULL}
/** 命名回调初始化，对于复杂对象，可能本就是一个结构体，可以使用回调返回该成员的反射引用数据 */
#define REFLECT_NAMECBK(name, reftype, objtype, member, handle)	\
	{ sizeof(objtype), name, reftype, PTRMEMBER_OFFSET(objtype, member), PTRMEMBER_SIZEOF(objtype, member), handle}


/**从反射字段数据中使用名称查找字段反射*/
static INLINE reflect_p reflect_find(reflect_p flds, const char* fldname) {
	reflect_p o;
	int i = 0;
	if(flds && fldname) {
		while((o = (flds + i)) && o->fldname && (REFLECT_NONE != o->type) && (o->size)) {
			if(!strcmp(fldname, o->fldname)) { return o; }
			i++;
		}
	}
	return NULL;
}


static INLINE int64_t reflect_getintvalue(reflect_p o, const void *p) {
	int64_t ret = 0;
	if(o && p && (REFLECT_INT == o->type)){
		switch(o->size) {
		case 1: ret =  *(int8_t*)(((char*)p) + o->offset); break;
		case 2: ret =  *(int16_t*)(((char*)p) + o->offset); break;
		case 4: ret =  *(int32_t*)(((char*)p) + o->offset); break;
		case 8: ret =  *(int64_t*)(((char*)p) + o->offset); break;
		default: break;
		}
	}
	return ret;
}
static INLINE uint64_t reflect_getuintvalue(reflect_p o, const void *p) {
	uint64_t ret = 0;
	if(o && p && (REFLECT_UINT == o->type || REFLECT_TIME == o->type)){
		switch(o->size) {
		case 1: ret =  *(uint8_t*)(((char*)p) + o->offset); break;
		case 2: ret =  *(uint16_t*)(((char*)p) + o->offset); break;
		case 4: ret =  *(uint32_t*)(((char*)p) + o->offset); break;
		case 8: ret =  *(uint64_t*)(((char*)p) + o->offset); break;
		default: break;
		}
	}
	return ret;
}
static INLINE double reflect_getfloatvalue(reflect_p o, const void *p){
	double ret = 0;
	if(o && p && (REFLECT_FLOAT == o->type)){
		if(sizeof(double) == o->size) {
			ret = *((double*)(((char*)p) + o->offset));
		} else if(sizeof(float) == o->size) {
			ret = *((float*)(((char*)p) + o->offset));
		}
	}
	return ret;
}
static INLINE const char* reflect_getstring(reflect_p o, const void *p){
	if(o && p){
		switch(o->type) {
		case REFLECT_STRBUF: return ((const char*)(((char*)p) + o->offset));
		case REFLECT_STRING: return *((const char**)(((char*)p) + o->offset));
		default: return NULL;
		}
	}
	return NULL;
}





static INLINE uint32_t reflect_setintvalue(reflect_p o, void* p, int64_t v) {
	if(o && p && (REFLECT_INT == o->type)){
		switch(o->size) {
		case 1: *(int8_t*)(((char*)p) + o->offset)	= (int8_t)v; return o->size;
		case 2: *(int16_t*)(((char*)p) + o->offset)	= (int16_t)v; return o->size;
		case 4: *(int32_t*)(((char*)p) + o->offset)	= (int32_t)v; return o->size;
		case 8: *(int64_t*)(((char*)p) + o->offset)	= (int64_t)v; return o->size;
		default: break;
		}
	}
	return 0;
}
static INLINE uint32_t reflect_setuintvalue(reflect_p o, void* p, uint64_t v){
	if(o && p && (REFLECT_UINT == o->type || REFLECT_TIME == o->type)){
		switch(o->size) {
		case 1: *(uint8_t*)(((char*)p) + o->offset)		= (uint8_t)v; return o->size;
		case 2: *(uint16_t*)(((char*)p) + o->offset)	= (uint16_t)v; return o->size;
		case 4: *(uint32_t*)(((char*)p) + o->offset)	= (uint32_t)v; return o->size;
		case 8: *(uint64_t*)(((char*)p) + o->offset)	= (uint64_t)v; return o->size;
		default: break;
		}
	}
	return 0;
}
static INLINE uint32_t reflect_setfloatvalue(reflect_p o, void *p, double v){
	if(o && p && (REFLECT_FLOAT == o->type)){
		if(sizeof(double) == o->size) {
			*((double*)(((char*)p) + o->offset)) = v;
		} else if(sizeof(float) == o->size) {
			*((float*)(((char*)p) + o->offset)) = (float)v;
		} else {
			return 0;
		}
		return o->size;
	}
	return 0;
}
static INLINE uint32_t reflect_setstring(reflect_p o, void *p, const char* v){
	if(o && p){
		switch(o->type) {
		case REFLECT_STRBUF: strncpy((char*)(((char*)p) + o->offset), v, o->size); return 1;
		case REFLECT_STRING: *((const char**)(((char*)p) + o->offset)) = v; return 1;
		default: return 0;
		}
	}
	return 0;
}

static INLINE const char* reflect_stringalloc(const char* v, uint32_t* vl) {
	void* sptr;
	uint32_t sln = 0;
	if(v && (sln = strlen(v)) && (sptr = calloc(1, sln + 1))){
		memcpy(sptr, v, sln);
		if(vl){ *vl = sln; }
	}
	return (const char*)sptr;
}
static INLINE uint32_t reflect_mallocstring(reflect_p o, void *p, const char* v){
	if(o && p){
		switch(o->type) {
		case REFLECT_STRBUF: strncpy( (char*)(((char*)p) + o->offset), v,o->size); return 1;
		case REFLECT_STRING: *((const char**)(((char*)p) + o->offset)) = reflect_stringalloc(v, NULL); return 1;
		default: return 0;
		}
	}
	return 0;
}


#ifdef __cplusplus
};
#endif

#endif