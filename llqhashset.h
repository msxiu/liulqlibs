/*
 * llqhashset.h
 *  Created on: 2016年1月14日
 *  版权:刘龙强
 *	哈希映射操作函数
 */

#ifndef LLQPAGER_H_
#define LLQPAGER_H_
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif


typedef struct hashelement_struct{//哈希元素结构体
    void* key;//键名
    void* val; //键值
	time_t time;//加入队列的时间
    struct hashelement_struct *next;       /*下个节点*/
} hashelement_t, *hashelement_p;

typedef int (*hashmapkey_compare)(const void* a, const void* b);//比较哈希key是否相等
typedef void (*hashmapkey_bind)(hashelement_p o, const void* b);//绑定哈希key的值
typedef uint16_t (*hashmapkey_calculate)(const void* b, uint32_t msize);//计算key的哈希值的函数
typedef void (*hashmapval_destory)(void* p);//哈希值的子属性数据回收

typedef struct hashmap_struct{//哈希操作结构体
    int size;//大小
    int count;//记录个数
	pthread_mutex_t			lock;//互斥锁
	hashmapkey_compare		compare;//比较函数;
	hashmapkey_bind			bindkey;//绑定键名到对象
	hashmapkey_calculate	hashkey;//计算哈希值的函数
	hashmapval_destory 		destoryval;//回收对象的下属数据
	hashelement_p 			table[];//哈希数组
} hashmap_t, *hashmap_p;


/**指定回调函数创建哈希映射对象 */
static INLINE hashmap_p hashmap_create(
	int size,		//哈希映射对象的最大长度
	hashmapkey_compare compare,			//哈希键比较函数
	hashmapkey_calculate hashkey,		//生成哈希键函数
	hashmapkey_bind bindkey				//绑定哈希键函数
) {
	int hsize = sizeof(hashmap_t) + (size * sizeof(void*));
    hashmap_p o;
    o = (hashmap_p)calloc(hsize, 1);
    o->size = size;//hash大小
    o->compare = compare;//hash比较函数
    o->hashkey = hashkey;//计算哈希值
    o->bindkey = bindkey;//绑定键名
    return o;
}
static INLINE void hashmap_setdestory(//设置子属回收函数
	hashmap_p o,			//哈希映射对象
	hashmapval_destory func	//回收函数
){
	o->destoryval = func;
}

static INLINE void hashelement_reclaim(hashelement_p p) {
	if(p->key) {
		free(p->key);
		p->key = NULL;
	}
	if(p->val) {
		free(p->val);
		p->val = NULL;
	}
	free(p);
	p = NULL;
}
typedef int (*hashelement_callback)(void*o, hashelement_t* b);//哈希表中的元素回调

//*******************************************************

/**从哈希表中查找键为key的数据 @return 找到返回数据地址,否则返回NULL  */
static INLINE void* hashmap_find(
	hashmap_t* o,		//哈希映射对象
	const void* key		//查找键名
) {
	uint16_t hash = o->hashkey(key, o->size);
	hashelement_t *p =  (hashelement_t*)o->table[hash];
    while (p && !o->compare(p->key, key))  p = p->next;
    return (p ? p->val : NULL);
}
/**判断哈希表中是否包含key的数据*@return {1:包含,0:不包含} */
static INLINE int hashmap_contains(
	hashmap_t* o,		//哈希映射对象
	const void* key		//查找键名
){
	uint16_t hash = o->hashkey(key, o->size);
	hashelement_t *p =  (hashelement_t*)o->table[hash];
    while (p && !o->compare(p->key, key))  p = p->next;
    return  (!!p);
}
/**判断哈希映射对象中键名为k的键值是否等于v*/
static INLINE int hashmap_equals_int32(
	hashmap_t* o,	//哈希映射对象
	void* k,		//键名
	int v			//比较的键值
){
	int* hv = (int*)hashmap_find(o, k);
	if(NULL == hv) return 0;
	return (v == *hv);
}
/**判断哈希映射对象中键名为k的键值是否等于v*/
static INLINE int hashmap_equals_string(
	hashmap_t* o, 	//哈希映射对象
	void* k, 		//键名
	char* v			//比较的键值
)//哈希值等于
{
	char* hv = (char*)hashmap_find(o, k);
	if(NULL == hv) return 0;
	return (0 == strcasecmp(hv, v) ? 1 : 0);
}


/**通过key删除哈希映射对象中的元素,并回收value中的内存数据*/
static INLINE int hashmap_delitem(
	hashmap_t* o,	//哈希映射对象
	const void* key	//键名
){
	hashelement_t *p0 = NULL, *p = NULL;
	int result = 0;
	uint16_t hash = o->hashkey(key, o->size);
    p = o->table[hash];
	pthread_mutex_lock(&(o->lock));
    while (p && !o->compare(p->key, key)) {
        p0 = p;
        p = p->next;
    }
    if (p) {
		if (p0) {//非第一个元素
			p0->next = p->next;
		} else {//第一个元素
			o->table[hash] = p->next;
		}
		if(o->destoryval){o->destoryval(p);}
		hashelement_reclaim(p);
		o->count--;
    }
	pthread_mutex_unlock(&(o->lock));
    return result;
}

/**返回哈希映射对象中的元素个数 */
static INLINE int hashmap_count(hashmap_t* o){//记录条数
	return o->count;
}

/**循环枚举哈希映射对象中的元素 */
static INLINE int hashmap_each(
	hashmap_t *o,		//哈希映射对象
	int (*callback)(void*o, hashelement_t* b),	//枚举元素前的回调函数
	void* arg			//调用回调的函数参数
) {
	int i=0, ret =0;
	hashelement_t *p0, *p;//
	for(;i<o->size;i++) {
		   p = o->table[i];
		    while (p) {
		        p0 = p;
		        (void)(callback && callback(arg, p));
		        ret ++;
		        p = p0->next;
		    }
	}
	return ret;
}

/**从哈希映射对象移除超时元素 */
static INLINE int hashmap_timeout(
	hashmap_t* o,	//哈希映射对象
	int second,		//超时的秒数
	hashelement_callback callback,	//删除元素前的回调函数
	void* arg		//调用回调的函数参数
){
	int i=0, ret =0;
	hashelement_t *p0, *p;
	time_t outime = time(NULL) - second;
	for(;i<o->size;i++) {
		   p = o->table[i];
		    while (p) {
		        p0 = p->next;
		        if(p->time < outime) {
		        	(void)(callback && callback(arg, p));
					pthread_mutex_lock(&(o->lock));
					if (p == o->table[i]) {//第一个元素
						o->table[i] = p->next;
					}
					if(o->destoryval){o->destoryval(p);}
					hashelement_reclaim(p);
					o->count--;
					pthread_mutex_unlock(&(o->lock));
					ret ++;
		        }
		        p = p0;
		    }
	}
	return ret;
}

/**销毁哈希映射对象 */
static INLINE void hashmap_destory(
	hashmap_t* o,					//哈希映射对象
	hashelement_callback callback,	//元素的回调函数
	void* arg
) {
	int i=0;
	hashelement_t *p0, *p;
	for(;i<o->size;i++) {
		   p = o->table[i];
		    while (p) {
		        p0 = p->next;
		        (void)(callback && callback(arg, p));
				if(o->destoryval){o->destoryval(p);}
		        hashelement_reclaim(p);
		        p = p0;
		    }
	}
	free(o);
}





static INLINE int hashmap_makekeyvalue(hashmap_t* o, hashelement_p top, uint16_t hash, const void* key, void* val){
	hashelement_p p = (hashelement_p)calloc(1, sizeof(hashelement_t));
	p->time = time(NULL);
	o->bindkey(p, key);
	p->val = val;
	if(top) p->next = top;
	o->table[hash] = p;
	o->count++;
	return 1;
}
/**给哈希映射对象添加数据元素 *@return 添加是否成功,如果存在则返回失败{1:成功,0:失败} */
static INLINE int hashmap_additem(
	hashmap_t* o,		//哈希映射对象
	const void* key,	//键名
	const void* val,	//键值数据
	uint32_t vln		//键值数据长度
) {
	if(NULL == key || NULL == val || 0 == vln) return 0;
	uint16_t hash = o->hashkey(key, o->size);
	hashelement_p top, p = o->table[hash];
	int result = 0;
	top = p;
    while (p && !o->compare(p->key, key))  p = p->next;
	pthread_mutex_lock(&(o->lock));
    if (!p) {
    	void* ptr = calloc(vln, 1);
    	memcpy(ptr, val, vln);
    	result = hashmap_makekeyvalue(o, top, hash, key, ptr);
     } else {
		free(p->val);
    	p->val = calloc(vln, 1);
    	memcpy(p->val, val, vln);
     }
	pthread_mutex_unlock(&(o->lock));
	return result;
}
/**给哈希映射对象添加数据元素,val是malloc出来的,且不能释放 *@return 添加是否成功,如果存在则返回失败{1:成功,0:失败} */
static INLINE int hashmap_addpointer(
	hashmap_t* o,		//哈希映射对象
	const void* key,	//键名
	void* val			//键值,由哈希映射对象负责内存释放,需要指子对象回收方法
) {
	if(NULL == key || NULL == val) return 0;
	uint16_t hash = o->hashkey(key, o->size);
	hashelement_t *top, *p = o->table[hash];
	int result = 0;
	top = p;
    while (p && !o->compare(p->key, key))  p = p->next;
	pthread_mutex_lock(&(o->lock));
    if (!p) {
    	result = hashmap_makekeyvalue(o, top, hash, key, val);
     } else {
		free(p->val);
    	p->val = val;
     }
	pthread_mutex_unlock(&(o->lock));
	return result;
}

/**向哈希表中添加键值为字符串类型的数据 *@return 添加是否成功,如果存在则返回失败{1:成功,0:失败} */
static INLINE int hashmap_addstring(
	hashmap_t* o,			//哈希映射对象
	const void* key,		//键名
	const char * const val	//添加的键值数据
) {
	return hashmap_additem(o, key, val, (NULL == val ? 0 : strlen(val)));
}
/**向哈希表中添加键值为整型的数据 *@return 添加是否成功,如果存在则返回失败{1:成功,0:失败} */
static INLINE int hashmap_addint32(
	hashmap_t* o,		//哈希映射对象
	const void* key,	//键名
	int val				//添加的键值数据
) {
	return hashmap_additem(o, key, &val, sizeof(int));
}
/**向哈希表中添加键值为长整型的数据 *@return 添加是否成功,如果存在则返回失败{1:成功,0:失败} */
static INLINE int hashmap_addint64(
	hashmap_t* o,		//哈希映射对象
	const void* key,	//键名
	int64_t val			//添加的键值数据
) {
	return hashmap_additem(o, key, &val, sizeof(int64_t));
}


#endif /* LLQPAGER_H_ */
