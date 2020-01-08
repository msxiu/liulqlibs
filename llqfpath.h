/*
 * llqfspath.h
 *
 *  Created on: 2017-7-24
 *      Author: Administrator
 */

#ifndef LLQFSPATH_H_
#define LLQFSPATH_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#if defined(WIN32)
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifndef INLINE
#ifdef WIN32	//windows平台下的宏定义
#define INLINE	__inline
#else
#define INLINE	inline
#endif	//---#if defined(WIN32)
#endif

#define PATH_SEPARATOR_UNIX		'/'
#define PATH_SEPARATOR_WIN		'\\'

/**返回目录分隔符*/
static INLINE const char fspath_getseparator(){
#ifdef WIN32
	return PATH_SEPARATOR_WIN;
#else
	return PATH_SEPARATOR_UNIX;
#endif
}

/**将一个地址规则化*/
static INLINE int fspath_criterion(char* s) {
	int i = 0;
	char src, dst;
#ifdef WIN32
	src = PATH_SEPARATOR_UNIX;
	dst = PATH_SEPARATOR_WIN;
#else
	src = PATH_SEPARATOR_WIN;
	dst = PATH_SEPARATOR_UNIX;
#endif
	while(*(s+i)) {
		if(src == *(s+i)) *(s+i)= dst;
		i++;
	}
	return i;
}
/**判断字符是否为分隔字符*/
static INLINE int fspath_separatorof(const int c) {
	return ((c == PATH_SEPARATOR_UNIX) || (c == PATH_SEPARATOR_UNIX));
}
/**吃掉路径尾部的分隔符*/
static INLINE int fspath_taileatseparator(char* s) {
	if(s){
		char l=strlen(s)-1;
		if(l>0 && fspath_separatorof(*(s+l))){
			*(s+l)=0;
			return 1;
		}
	}
	return 0;
}
/***判断地址是否为绝对地址*/
static INLINE int fspath_absolute(const char* s) {
#ifdef WIN32
	return (s && ':' == *(s + 1) && fspath_separatorof(*(s + 2)));
#else
	return fspath_separatorof(*s);
#endif
	//return (fspath_separatorof(*s) || (':' == *(s + 1) && fspath_separatorof(*(s + 2))));
}

/**获得路径的文件名*/
static INLINE char* fspath_filename(char* s) {
	char c;
	int p = 0, i =0;
	if(s) {
		while((c = *(s + i))) {
			if(c == PATH_SEPARATOR_UNIX || c == PATH_SEPARATOR_WIN) p = i;
		}
		return (0 == p ? s : s + p + 1);
	}
	return NULL;
}

/**文件路径截取,截取最后一个/之前的路径, return 返回地址的上一级目录,尾部带/结尾 */
static INLINE char* fspath_cutpart(
	char* addr,		//路径地址
	int endremove	//{1:如果最后一个字符以/结尾是否需要移除,2:是否清除尾部的/字符}
) {
	char ch;
	int sln = (NULL == addr ? 0 : strlen(addr) - 1);
	if((endremove & 1) && sln && fspath_separatorof(*(addr + sln))) {
		*(addr+(sln--)) = 0;//吃掉尾部的分隔符
	}
	while(sln && (ch = *(addr + sln))) {
		if(fspath_separatorof(ch)) {
			if((endremove & 2)) { *(addr+sln) = 0; }
			break;
		}
		*(addr+sln) = 0;
		sln--;
	}
	return addr;
}

static INLINE int fspath_adjust_typeof(const char* s) {
	if(s) {
		if(('.' == *s)) {
			if(('.' == *(s+1)) && fspath_separatorof(*(s+2))) {//[/../]这种形式
				return 3;
			} else if(fspath_separatorof(*(s+1))) {//[/./]这种形式
				return 2;
			}
		} else if(fspath_separatorof(*s)){
			return 1;
		}
	}
	return 0;
}
/**对目录地址进行调整,去掉.与..的映射*/
static INLINE char* fspath_adjust(
	char* target
) {
	int sln, p1 = 0, p2 = 0, tid = 0;
	char c;
	if(target) {
		sln = strlen(target);
		while((c = *(target+p1))) {
			if(fspath_separatorof(c)){
				if((tid = fspath_adjust_typeof(target+p1+1))) {
					switch(tid){
					case 1://[//]
						memmove(target + p1, target + p1 + 1, (sln - p1));//吃掉一字节
						p1--;
						break;
					case 2://[/./]
						memmove(target + p1, target + p1 + 2, (sln - p1 - 1));//吃掉两字节
						p1--;
						break;
					case 3://[/../]
						if(p2 == p1){
							while(p2--&&!fspath_separatorof(*(target + p2)));//找到上一分隔符
						}
						memmove(target + p2, target + p1 + 3, (sln - p1 - 2));//吃掉字符到上一分隔符处
						p1=(p2 - 1);
						break;
					default:break;
					}
				} else {
					p2 = p1;
				}
			}
			p1++;
		}
	}
	return target;
}

/**文件路径映射, return 返回生成后的新地 ,错误返回NULL*/
static INLINE char* fspath_map(
	char* target,		//映射完后的返回地址
	const char*baddr,	//基本地址,以/结尾的
	const char* taddr	//目标地址
) {
	int sln = 0;//, i = 0;
	char* kaddr, *tmpaddr;
	if(target && baddr && taddr) {
		char isb = (NULL == baddr || !(*baddr)), ist = (NULL == taddr || !(*taddr));
		if(isb || isb) {
			if(isb && !ist) {
				strcpy(target, taddr);
				return target;
			} else if(ist && !isb) {
				strcpy(target, baddr);
				return target;
			}
			return NULL;//两者同时回空,返回NULL
		}
		if(!(kaddr = (char*)calloc(1, strlen(baddr) + strlen(taddr) + 2))){//生成目标目录的缓存
			return NULL;
		}
		if(!(tmpaddr = (char*)calloc(1, strlen(baddr) + 10))) {//复制
			free(kaddr);
			return NULL;
		}
		strcpy(tmpaddr, baddr);

		if(fspath_absolute(taddr)) {//表示目标就是绝对地址
			strcpy(kaddr, taddr);
		} else if('.' == *taddr && fspath_separatorof(*(taddr+1))){
			sln = strlen(tmpaddr);
			if(!fspath_separatorof(*(tmpaddr + (sln - 1)))) {//不是以/结尾,表示最后一部分是文件名
				fspath_cutpart(tmpaddr, 1);
			}
			strcpy(kaddr, tmpaddr);
			strcat(kaddr, taddr+2);
		} else {
			while('.' == *taddr && '.' == *(taddr+1) && fspath_separatorof(*(taddr+2))) {
				fspath_cutpart(tmpaddr, 1);
				taddr += 3;
			}
			strcpy(kaddr, tmpaddr);
			if('.' == *taddr) {
				strcat(kaddr, taddr+2);
			} else {
				strcat(kaddr, taddr);
			}
		}
		strcpy(target, kaddr);
		fspath_criterion(target);
		free(tmpaddr);
		free(kaddr);
	}

	return target;
}

/**将参数格式化后,生成目录地址*/
static INLINE char* fspath_mapformat(
	char* dst,				//目标地址
	const char* root,		//根地址
	const char* fmt, ...	//格式化参数
) {
	char src[1024];
	va_list ap;
	if(dst && root && fmt) {
		memset(src, 0, sizeof(src));
		va_start(ap, fmt);
		vsnprintf(src, sizeof(src), fmt, ap);
		fspath_map(dst, root, src);
		va_end(ap);
	}
	return dst;
}


/**映射绝对工作路径, return 映射后的目录绝对路径保存地址  */
static INLINE char* fspath_mapworkfolder(
	char* target,	//映射后的目录绝对路径保存地址
	const char* cmd	//启动的命令地址
) {
	char path_cmd_start[1024];
	int sln;
	if(cmd && target) {
		getcwd(path_cmd_start, sizeof(path_cmd_start));//获取当前工作目录
		sln = strlen(path_cmd_start);
		if(!fspath_separatorof(*(path_cmd_start+(sln - 1)))) {
    		*(path_cmd_start + sln) = fspath_getseparator();//赋值最后一个字符为分隔符
		}
		fspath_map(target, path_cmd_start, cmd);
		return fspath_cutpart(target, 1);
	}
	return NULL;
}




static INLINE int fspath_isdirectory(const char *path) {//判断地址是否为目录
#if defined(WIN32)
	struct _stat buf = {0};  
	if(path && 0 == _stat(path, &buf)) {
		return ((buf.st_mode & _S_IFDIR) ? 1: 0);
	}
#else
	struct stat buf = {0};
	if(path && 0 == stat(path, &buf)) {
		return ((buf.st_mode & __S_IFDIR) ? 1: 0);
	}
#endif
	return 0;
}

/**多级目录创建, return 创建成功返回1,失败返回0 */
static INLINE int fspath_create(
	const char *path	//目录地址
#if defined(WIN32)
#else
	, int mode			//权限
#endif
) {
	int i, ret = 1, len;
	char *dir_path = NULL;
	if(path && !fspath_isdirectory(path)) {//地址不为空,同时地址不存在
		ret = 0;
		len = strlen(path);
		dir_path = (char*)calloc(1, len+1);
		memset(dir_path, 0, len);
		strncpy(dir_path, path, len);
#ifdef WIN32
		for (i=3; i<len; i++) {
#else
		for (i=1; i<len; i++) {
#endif
			if (fspath_separatorof(dir_path[i]) && i > 0) {
				dir_path[i]='\0';
				if (!fspath_isdirectory(dir_path)) {
#if defined(WIN32)
					if (mkdir(dir_path) < 0)
#else
					if (mkdir(dir_path, mode) < 0)
#endif
					{
						goto LABEXIT;
					}
				}
				dir_path[i]=fspath_getseparator();
			}
		}
		if(!fspath_separatorof(*(path + len -1)) && !fspath_isdirectory(path)) {
#if defined(WIN32)
			if (mkdir(dir_path) < 0)
#else
			if (mkdir(dir_path, mode) < 0)
#endif
			{
				goto LABEXIT;
			}
		}
		ret = fspath_isdirectory(path);
	}
	return ret;

LABEXIT:
	if(dir_path) free(dir_path);
	return 0;
}
//
/**根据指定文件,创建相关目录, return 返回创建目录级数  */
static INLINE int fspath_createwithfile(
	const char *faddr		//文件地址
) {
	int ret = 0;
	char* path;
	if(faddr && (path = (char*)calloc(1, strlen(faddr) + 1))) {
		strcpy(path, faddr);
		fspath_cutpart(path, 2);
#if defined(WIN32)
		ret = fspath_create(path);
#else
		ret = fspath_create(path, 0755);
#endif
		free(path);
	}
	return ret;
}

#endif /* LLQFSPATH_H_ */
