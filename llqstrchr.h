#ifndef LLQ_STRCHR_H_
#define LLQ_STRCHR_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif


/**比较两个字母是否相等*/
static INLINE int strchrcompare(wchar_t ac, wchar_t bc) {
	return (ac - bc);
}
/**忽略大小写比较两个字母是否相等*/
static INLINE int strchrcasecompare(wchar_t ac, wchar_t bc) {
	switch(((isupper(ac) ? 1 : 0) | (isupper(bc) ? 2 : 0))){//仅在他们大小写不一致的情况下转小写
	case 1:ac+=32;break;
	case 2:bc+=32;break;
	default:break;
	}
	return (ac - bc);
}
/**查找字符c在字符串s中出现的位置, @return c在p中的开始位置,没有返回-1 */
static INLINE int strchrfind(const char* str,const char c, size_t pos) {
	char ch;
	if(str) {
		while((ch = *(str+pos))) {
			if(ch == c) return pos;
			pos++;
		}
	}
	return -1;
}
/**查找字符c在字符串p中出现的位置, @return c在p中的开始位置,没有返回-1 */
static INLINE int strchrindexof(const char* p, const char c) {
	return strchrfind(p, c, 0);
}
/**从尾部查找字符串str中包含字符c的位置, @return c在str中的位置 */
static INLINE int strchrlastindexof(const char* str, char c, int pos) {
	int idx = -1, i=0, ch;
    if(str) {
		while((i < pos || -1 == pos) && (ch = *(str+i))) {
            if(ch == c) idx = i;
            i++;
		}
	}
	return idx;
}

/**比较字符串s1是否s2开始,返回比较长度 */
static INLINE int strchrstartof(const char* str, const char* sub, int(*compare)(wchar_t, wchar_t)) {
	int i=0;
	unsigned char ch;
	const unsigned char *a = (const unsigned char *)str,  *b = (const unsigned char *)sub;;
	if(a && b) {
		while((ch = *(b + i))) {
			if(compare(*(a+i), ch)) return 0;
			i++;
		}
		return (0 == ch ? i : 0);//ch等于0表示比较到了尾部
	}
	return 0;
}
static INLINE int strchrstartwith(const char* str, const char* sub) {
	return strchrstartof(str, sub, strchrcompare);
}
static INLINE int strchrcasestartwith(const char* str, const char* sub) {
	return strchrstartof(str, sub, strchrcasecompare);
}

/**查找字符c在字符串p中出现的位置,没有返回-1 */
static INLINE int strchs_indexof(
	const char* src,	//原始字符串
	const char* p,		//包含字符串
	int *cln,			//接收比较字符串p长度的指针
	int(*compare)(wchar_t, wchar_t)	//比较函数
) {
	int i=0, ps;
	unsigned char ch, c = (unsigned char)(*p);
	const unsigned char* s = (const unsigned char*)src;
	if(src && p) {
		while((ch = *(s+i))) {
			if(!compare(ch, c) && (ps = strchrstartof(src + i, p, compare))) {
				if(cln) { *cln = ps; }
				return i;
			}
			i++;
		}
	}
	return -1;
}
/**查找b在a中出现的位置, @return 如果找到返回出现b的开始位置,否则返回-1 */
static INLINE int strchsindexof(const char* str, const char* sub){
	return strchs_indexof(str, sub, NULL, strchrcompare);
}
/**忽略大小写查找b在a中出现的位置, @return 如果找到返回出现b的开始位置,否则返回-1 */
static INLINE int strchscaseindexof(const char* str, const char* sub){
	return strchs_indexof(str, sub, NULL, strchrcasecompare);
}
/**查找b在a中出现的位置, @return 如果找到返回出现b的结束位置,否则返回-1 */
static INLINE int strchsindextail(const char* str, const char* sub) {
	int l=0, i= strchs_indexof(str, sub, &l, strchrcompare);
	return (i + l);
}
/**忽略大小写查找b在a中出现的位置, @return 如果找到返回出现b的结束位置,否则返回-1 */
static INLINE int strchscaseindextail(const char* str, const char* sub){
	int l=0, i= strchs_indexof(str, sub, &l, strchrcasecompare);
	return (i + l);
}



/**从尾部查找字符串str中包含字符c的位置, @return c在str中的位置 */
static INLINE int strchs_lastindexof(
	const char* src,	//被查找字符串
	const char* cstr,	//查找的字符
	int p,				//开始查找位置,从最尾部开始赋值-1
	int *cln,			//
	int(*compare)(wchar_t, wchar_t) //字符比较函数
) {
	int rln = 0;
	unsigned char c;
	if(src && cstr) {
		c = (unsigned char)(*cstr);
		if(-1 == p) { p = strlen(src) - 1; }
		while(p > -1) {
			if((0 == compare(*(src+p), c)) && (rln = strchrstartof(src + p, cstr, compare))) {
				if(cln) { *cln = rln; }
				return p;
			}
			p--;
		}
	}
	return -1;
}
/**查找b在a中出现的位置, @return 如果找到返回出现b的开始位置,否则返回-1 */
static INLINE int strchslastindexof(const char*a, const char* b, int offset){
	return strchs_lastindexof(a, b, offset, NULL, strchrcompare);
}
/**忽略大小写查找b在a中出现的位置, @return 如果找到返回出现b的开始位置,否则返回-1 */
static INLINE int strchslastcaseindexof(const char*str, const char* sub, int offset){
	return strchs_lastindexof(str, sub, offset, NULL, strchrcasecompare);
}
/**查找b在a中出现的位置, @return 如果找到返回出现b的结束位置,否则返回-1 */
static INLINE int strchslastindextail(const char* str, const char* sub, int offset){
	int l=0, i= strchs_lastindexof(str, sub, offset, &l, strchrcompare);
	return (i + l);
}
/**忽略大小写查找b在a中出现的位置, @return 如果找到返回出现b的结束位置,否则返回-1 */
static INLINE int strchslastcaseindextail(const char*str,	const char* sub, int offset){
	int l=0, i= strchs_lastindexof(str, sub, offset, &l, strchrcasecompare);
	return (i + l);
}


#endif