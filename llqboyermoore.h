/*
 * llqboyermoore.h
 *  Created on: 2017-12-27
 *      Author: Administrator
 */

#ifndef LLQBOYERMOORE_H_
#define LLQBOYERMOORE_H_
#include <string.h>

#define MAX(a, b)				(((a)>(b)) ? (a) : (b))
//最大字符个数
#define MAX_CHAR				256
//关键词的最大长度
#define MAX_LENGTH				256

typedef struct {
	const unsigned char* key;
	int kln;
	unsigned char bc[MAX_CHAR];
	unsigned char gs[MAX_CHAR];
} boyermoore_t, * boyermoore_p;
#define BOYERMOORE_EMPTY		{NULL, 0, {0}, {0}}

/* 计算坏字符(即尾字符的不匹配字符)的跳过字符个数数组 */
static INLINE void boyermoore_badcharacter(
	boyermoore_p o		//BM算法对象
) {
	int i;
    unsigned char ch;
	for (i = 0; i < MAX_CHAR; i++) {
		o->bc[i] = o->kln;//搜索词中不存在坏字符,跳过长度为关键词的长度
	}
    for (i = 0; i < o->kln; i++) {
		ch = *(o->key + i);//取到当前字符
		o->bc[ch] = o->kln - 1 - i;//搜索词中有对应的坏字符
	}
}

/* 计算后缀数组 */
static INLINE void boyermoore_suffixes(
	const unsigned char* p,	//关键词地址
	int m,			//关键词的长度
	int suff[]		//好后缀缓存区
) {
	int i;
    int f	= 0;		//关键词查询的当前下标,
    int g	= m - 1;	//搜索词中,字符上一次出现位置
	int ng	= g;		//关键词中字符的最大下标
    suff[ng] = m;
    for (i = m - 2; i >= 0; i--) {
        if (i > g && suff[ng - f + i] < i - g) {
            suff[i] = suff[ng - f + i];
		} else {
            if (i < g) {
				g = i;
			}
            f = i;//赋值查找下标
            while (g >= 0 && p[g] == p[ng - f + g]) {
				g--;
			}
            suff[i] = f - g;
        }
    }
}
/* 计算好后缀数组 */
static INLINE void boyermoore_goodsuffixes(
	boyermoore_p o		//BM算法对象
) {
	int i, j = 0, ng = (o->kln - 1), ps = 0, suff[MAX_LENGTH];
	boyermoore_suffixes(o->key, o->kln, suff);
    for (i = 0; i < o->kln; i++) {//不存在和 "好后缀" 匹配的子串，则右移整个搜索词
        o->gs[i] = o->kln;
	}
    for (i = o->kln - 2; i >= 0; i--) {//不存在和 "最长好后缀" 完全匹配的子串，则选取长度最长且也属于前缀的那个 "真好后缀"
        if (suff[i] == i + 1) {
            for (; j < ng - i; j++) {
                o->gs[j] = ng - i;
			}
		}
    }
    for (i = 0; i <= o->kln - 2; i++) {//有子串和 "最长好后缀" 完全匹配，则将最靠右的那个完全匹配的子串移动到 "最长好后缀" 的位置继续进行匹配
		ps = ng - suff[i];
        o->gs[ps] = ng - i;
	}
}

static INLINE int boyermoore_compile(boyermoore_p o, const void* key) {
	if(o && key) {
		o->key = (const unsigned char*)key;
		o->kln = strlen((const char*)o->key);
		boyermoore_badcharacter(o);
		boyermoore_goodsuffixes(o);
		return 1;
	}
	return 0;
}

static INLINE unsigned int boyermoore_search(boyermoore_p o, const char* s, unsigned int *buffer, unsigned int bln) {
	unsigned int ret = 0;//返回值,为匹配到的关键词次数
    unsigned char ch;//后缀不同字符
	int j = 0;//查找位置
	int i;//后缀不同字符位置
	int si;//跳过的字符个数
	if(o && s && buffer && bln) {
		int n = strlen(s);//查找字符串的长度
		while (j <= n - o->kln) {
			for (i = o->kln - 1; i >= 0 && o->key[i] == (ch = s[i + j]); i--);//从后遍历关键词,确定当前位置是否包含关键词
			if (i < 0) {
				if(ret < bln) { buffer[ret++] = j; }
				si = o->gs[0];
			} else {
				si = MAX(o->bc[ch] - o->kln + 1 + i, o->gs[i]);
			}
			j += si;
		}
	}
	return ret;
}


#endif /* LLQBOYERMOORE_H_ */
