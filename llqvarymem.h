#ifndef LLQVARYMEM_H_
#define LLQVARYMEM_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif


#ifndef MEMREALLOC_ALIGN
#define MEMREALLOC_ALIGN		64
#endif
/***固定宽度对齐计算,@parameter{n:所需大小,bit:对齐宽度}*/
#ifndef WIDTH_ALIGNED
#define WIDTH_ALIGNED(n, m)		((((n) / m) * m) + m)
#endif


//**** vary memory ********************************************************************************************************
/**使用realloc增加内存,并清零扩大部分的内存区, 分配成功返回1,分配失败返回0 
 * address：双指针内存地址；fln：已分配内存大小；mln：目标内存大小 */
static INLINE int memrealloc(void** address,	uint32_t fln, uint32_t mln) {
	char* rbuf = NULL, *former;
	if(address && (mln > fln)) {
		if((former = (char*)(*address)) && fln) {
			if(!(rbuf = (char*)realloc(former, mln))) {
				if(former) {
					free(former);
					former = NULL;
				}
				return 0;
			}
			former = rbuf;
			memset(former + fln, 0, mln - fln);
			(*address) = (void*)former;
		} else {
			if(!(former = (char*)calloc(1, mln))) {
				return 0;
			}
			(*address) = (void*)former;
		}
	}
	return 1;
}
/** 获取下一个对象（指定长度）的地址,当超出大小时,重新改变内存大小 
 * ptr：缓存区双指针地址；length:已申请缓存区长度;offset:写入数据位置;vl:写入数据长度 */
static INLINE void* memrealloc_nextof(void** ptr, uint32_t *length, uint32_t *offset, uint32_t vl) {
	void* res = NULL;
	if(ptr && length && offset && vl){
		if(!(*ptr) || (*length) <= (*offset) + vl) {//内存不够大,重新分配
			uint32_t tln = ((*offset) + vl);//写入目标后的长度
			uint32_t mln = WIDTH_ALIGNED(tln, MEMREALLOC_ALIGN);//按指定宽度对齐后的长度
			uint32_t oln = *length;//以64字节对齐
			if(memrealloc(ptr, oln, mln)) {
				(*length) = mln;//重新申请内存成功,修改内存长度
			}
		}
		if((*length) >= (*offset) + vl) {
			res = (void*)(((unsigned char*)(*ptr)) + (*offset));
			(*offset) += vl;
		}
	}
	return res;
}

/** 向标明大小缓存区对象写入数据,当超出大小时,重新改变内存大小 
 * ptr:缓存区双指针地址; size:已申请缓存区长度; offset:写入数据位置; v:写入数据地址; vl：写入数据长度
*/
static INLINE uint32_t memrealloc_write(void** ptr, uint32_t *size, uint32_t *offset, const void* v, uint32_t vl) {
	if(ptr && size && offset && v && vl) {
		void* res = NULL;
		if((res = memrealloc_nextof(ptr, size, offset, vl))) {
			memcpy(res, v, vl);
			return vl;
		}
	}
	return 0;
}
//动态内存使用该组函数作写操作,读操作调用binbuffer_提供的相关函数操作
static INLINE uint32_t memrealloc_writeint8(void** o, uint32_t *size, uint32_t *offset,int8_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(int8_t));
}
static INLINE uint32_t memrealloc_writeint16(void** o, uint32_t *size, uint32_t *offset,int16_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(int16_t));
}
static INLINE uint32_t memrealloc_writeint32(void** o, uint32_t *size, uint32_t *offset,int32_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(int32_t));
}
static INLINE uint32_t memrealloc_writeint64(void** o, uint32_t *size, uint32_t *offset,int64_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(int64_t));
}

static INLINE uint32_t memrealloc_writeuint8(void** o, uint32_t *size, uint32_t *offset,uint8_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(uint8_t));
}
static INLINE uint32_t memrealloc_writeuint16(void** o, uint32_t *size, uint32_t *offset,uint16_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(uint16_t));
}
static INLINE uint32_t memrealloc_writeuint32(void** o, uint32_t *size, uint32_t *offset,uint32_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(uint32_t));
}
static INLINE uint32_t memrealloc_writeuint64(void** o, uint32_t *size, uint32_t *offset,uint64_t v) {
	return memrealloc_write(o, size, offset, &v, sizeof(uint64_t));
}

static INLINE uint32_t memrealloc_writefloat(void** o, uint32_t *size, uint32_t *offset,float v){
	return memrealloc_write(o, size, offset, &v, sizeof(float));
}
static INLINE uint32_t memrealloc_writedouble(void** o, uint32_t *size, uint32_t *offset,double v){
	return memrealloc_write(o, size, offset, &v, sizeof(double));
}
static INLINE uint32_t memrealloc_writestring(void** o, uint32_t *size, uint32_t *offset, const char* s){
	if(s) {
		return memrealloc_write(o, size, offset, s, strlen(s) + 1);//写入尾部的空标记
	}
	return memrealloc_writeint8(o, size, offset, 0);//写入空标记
}


//**** vary size pointer ********************************************************************************************************
typedef struct {//固定大小的数据缓存区指针
	unsigned int size;//数据长度
	const void* ptr;//数据指针地址
}sizeptr_t, *sizeptr_p;
static INLINE void varysizeptr_reclaim(sizeptr_p o){
	if(o) {
		if(o->ptr) {free((void*)o->ptr);}
		memset(o, 0, sizeof(sizeptr_t));
	}
}
static INLINE int varysizeptr_initialize(sizeptr_p o, uint32_t *maxsz) {
	if(o) {
		memset(o, 0, sizeof(sizeptr_t));
		return (NULL != (o->ptr = calloc(*maxsz, 1)));
	}
	return 0;
}
/***动态申请内存写数据函数,使用该组方法写,使用chrbuffer_提供的读函数读*/
static INLINE uint32_t varysizeptr_write(sizeptr_p o, uint32_t *maxsz, const void* v, uint32_t vl) {
	if(o && maxsz && v && vl) {
		return memrealloc_write((void**)(&(o->ptr)), maxsz, &(o->size), v, vl);
	}
	return 0;
}
static INLINE uint32_t varysizeptr_writeint8(sizeptr_p o, uint32_t *size, int8_t v) {
	return varysizeptr_write(o, size, &v, sizeof(int8_t));
}
static INLINE uint32_t varysizeptr_writeint16(sizeptr_p o, uint32_t *size, int16_t v) {
	return varysizeptr_write(o, size, &v, sizeof(int16_t));
}
static INLINE uint32_t varysizeptr_writeint32(sizeptr_p o, uint32_t *size, int32_t v) {
	return varysizeptr_write(o, size, &v, sizeof(int32_t));
}
static INLINE uint32_t varysizeptr_writeint64(sizeptr_p o, uint32_t *size, int64_t v) {
	return varysizeptr_write(o, size, &v, sizeof(int64_t));
}

static INLINE uint32_t varysizeptr_writeuint8(sizeptr_p o, uint32_t *size, uint8_t v) {
	return varysizeptr_write(o, size, &v, sizeof(uint8_t));
}
static INLINE uint32_t varysizeptr_writeuint16(sizeptr_p o, uint32_t *size, uint16_t v) {
	return varysizeptr_write(o, size, &v, sizeof(uint16_t));
}
static INLINE uint32_t varysizeptr_writeuint32(sizeptr_p o, uint32_t *size, uint32_t v) {
	return varysizeptr_write(o, size, &v, sizeof(uint32_t));
}
static INLINE uint32_t varysizeptr_writeuint64(sizeptr_p o, uint32_t *size, uint64_t v) {
	return varysizeptr_write(o, size, &v, sizeof(uint64_t));
}

static INLINE uint32_t varysizeptr_writefloat(sizeptr_p o, uint32_t *size, float v){
	return varysizeptr_write(o, size, &v, sizeof(float));
}
static INLINE uint32_t varysizeptr_writedouble(sizeptr_p o, uint32_t *size, double v){
	return varysizeptr_write(o, size, &v, sizeof(double));
}
static INLINE uint32_t varysizeptr_writestr(sizeptr_p o, uint32_t *size, const char* s) {
	if(s) {
		return varysizeptr_write(o, size, s, strlen(s));
	}
	return 0;
}
static INLINE uint32_t varysizeptr_writestring(sizeptr_p o, uint32_t *size, const char* s) {
	if(s) {
		return varysizeptr_write(o, size, s, strlen(s) + 1);
	}
	return varysizeptr_writeuint8(o, size, 0);
}


/**** vary size pointer ********************************************************************************************************
*/
typedef struct {//指明大小\位置的数据缓存区
	unsigned int size;		//申请数据长度
	unsigned int offset;	//已使用
	const void* ptr;		//数据指针地址
} offsetptr_t, * offsetptr_p;
#define OFFSETPTR_EMPTY		{0, 0, NULL}

//向标明大小缓存区对象写入数据,当超出大小时,重新改变内存大小
static INLINE void offsetptr_reclaim(offsetptr_p o){
	if(o) {
		if(o->ptr) {free((void*)o->ptr);}
		memset(o, 0, sizeof(offsetptr_t));
	}
}
/**将o中的数据转移到dst,并清空o中数据*/
static INLINE void offsetptr_migrate(offsetptr_p o, offsetptr_p dst){
	if(o && dst) {
		offsetptr_reclaim(dst);
		memcpy(dst, o, sizeof(offsetptr_t));
		memset(o, 0, sizeof(offsetptr_t));
	}
}
static INLINE uint32_t offsetptr_write(offsetptr_p o, const void* v, uint32_t vl) {
	if(o && v && vl) {
		void* res = NULL;
		if((res = memrealloc_nextof((void**)(&(o->ptr)), &(o->size), (&o->offset), vl))) {
			memcpy(res, v, vl);
			return vl;
		}
	}
	return 0;
}
static INLINE uint32_t offsetptr_writeint8(offsetptr_p o, int8_t v) {
	return offsetptr_write(o, &v, sizeof(int8_t));
}
static INLINE uint32_t offsetptr_writeint16(offsetptr_p o, int16_t v) {
	return offsetptr_write(o, &v, sizeof(int16_t));
}
static INLINE uint32_t offsetptr_writeint32(offsetptr_p o, int32_t v) {
	return offsetptr_write(o, &v, sizeof(int32_t));
}
static INLINE uint32_t offsetptr_writeint64(offsetptr_p o, int64_t v) {
	return offsetptr_write(o, &v, sizeof(int64_t));
}

static INLINE uint32_t offsetptr_writeuint8(offsetptr_p o, uint8_t v) {
	return offsetptr_write(o, &v, sizeof(uint8_t));
}
static INLINE uint32_t offsetptr_writeuint16(offsetptr_p o, uint16_t v) {
	return offsetptr_write(o, &v, sizeof(uint16_t));
}
static INLINE uint32_t offsetptr_writeuint32(offsetptr_p o, uint32_t v) {
	return offsetptr_write(o, &v, sizeof(uint32_t));
}
static INLINE uint32_t offsetptr_writeuint64(offsetptr_p o, uint64_t v) {
	return offsetptr_write(o, &v, sizeof(uint64_t));
}

static INLINE uint32_t offsetptr_writefloat(offsetptr_p o, float v){
	return offsetptr_write(o, &v, sizeof(float));
}
static INLINE uint32_t offsetptr_writedouble(offsetptr_p o, double v){
	return offsetptr_write(o, &v, sizeof(double));
}
static INLINE uint32_t offsetptr_writestr(offsetptr_p o, const char* s){
	return offsetptr_write(o, s, strlen(s));
}
static INLINE uint32_t offsetptr_writestring(offsetptr_p o, const char* s) {
	if(s) {
		return offsetptr_write(o, s, strlen(s) + 1);
	}
	return offsetptr_writeuint8(o, 0);
}
/**吃掉头部指定长度的数据*/
static INLINE uint32_t offsetptr_eathead(offsetptr_p o, uint32_t ln) {
	if(o && ln && o->offset && o->offset >= ln) {
		if(o->offset == ln) {
			memset((void*)o->ptr, 0, o->offset);
			o->offset = 0;
		} else {
			int vln = (o->offset - ln);
			memmove((void*)o->ptr, ((char*)o->ptr)+ln, vln);
			memset(((char*)o->ptr)+ vln, 0, ln);
			o->offset -= ln;
		}
		return ln;
	}
	return 0;
}




/**** vary record set (可变对象数组)操作函数********************************************************************************************************
type_t ptr[n];是一个固定大小的记录组，(可变对象数组)中的记录数是可扩展的
*/
#ifndef SIZEPTR_MEMINCREASE
#define	SIZEPTR_MEMINCREASE		8
#endif
/** 向指针数据添加一个记录,添加成功返回下标,失败返回-1
 * count：申请记录总个数,重新申请后修改；idx：已使用记录个数,添加后自增；rsize：单个记录的大小*/
static INLINE void* varyrecordset_next(void **ptrs, uint32_t *count, uint32_t *idx, uint32_t rsize) {
	void* res = NULL;
	if((*idx) >= (*count)) {//超时记录时,需要重新申请内存
		if(memrealloc((void**)ptrs, ((*count) * rsize), (((*count) + SIZEPTR_MEMINCREASE) * rsize))){
			(*count) += SIZEPTR_MEMINCREASE;
		}
	}
	if((*idx) < (*count)) {
		res = (((char*)(*ptrs))+((*idx)*rsize));
		(*idx)++;
	}
	return res;
}
/*** 向指针数据添加一个记录,添加成功返回下标,失败返回-1 
 * item：添加记录地址；rsize：记录长度；count：申请记录总个数,重新申请后修改；idx：使用记录个数,添加后自增*/
static INLINE int varyrecordset_append(void **ptrs, void* item, uint32_t rsize, uint32_t *count, uint32_t *idx){
	int ret = (*idx);
	void* res = NULL;
	if((res=varyrecordset_next(ptrs, count, idx, rsize))) {
		memcpy(res, item, rsize);
		return ret;
	}
	return -1;
}
/** 获取记录地址
 * rsize：记录长度；count：申请记录总个数,重新申请后修改；idx:使用记录个数,添加后自增 */
static INLINE void* varyrecordset_get(const void *ptrs, uint32_t rsize, uint32_t count, uint32_t idx){
	if(ptrs && (idx < count)) {
		return (void*)(((char*)ptrs) + (idx * rsize));
	}
	return NULL;
}




/****	vary pointer list (可变指针数组)操作函数********************************************************************************************************
		void* ptr[n];表示一个固家大小的数据，(可变指针数组)的记录多少是可扩的
*/
/** 获取下个记录的指针地址 */
static INLINE void* varyptrlist_next(void** o, uint32_t* size,	uint32_t* offset) {
	void* res = NULL;
	uint32_t length = ((*size) * sizeof(void*)), position = ((*offset) * sizeof(void*));
	if((res = memrealloc_nextof(o, &length, &position, sizeof(void*)))) {
		*size	= (length / sizeof(void*));
		*offset	= (position / sizeof(void*));
	}
	return res;
}
/** 在尾部增加一个记录 */
static INLINE uint32_t varyptrlist_append(void** o,	uint32_t* size, uint32_t* offset, const void* v) {
	void* p = NULL;
	if((p = varyptrlist_next(o, size, offset))){
		*((const void**)p) = v;
		return sizeof(void*);
	}
	return 0;
}
/** 在指定下标位置加入一个记录 */
static INLINE uint32_t varyptrlist_insert(void** o, uint32_t* size, uint32_t* offset, uint32_t idx, const void* v) {
	void* p = NULL;
	if(o && size && offset && v && (p = varyptrlist_next(o, size, offset))){
		int dln = (((*offset) - idx) * sizeof(void*));//需要考贝的数据长度
		if(dln > 0){
			char *sptr = (char*)(*o);
			char *src = sptr + (idx * sizeof(void*));//数据移动的源数据位置
			char *dst = (char*)(src + sizeof(void*));//数据移动的目标数据位置
			memmove(dst, src, dln);//将源数据考贝到目标数据区
			*((const void**)src) = v;
		} else {
			*((const void**)p) = v;//直接在尾部添加
		}
		return sizeof(void*);
	}
	return 0;
}

#endif