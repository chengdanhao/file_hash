#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>
#include "hash.h"

#define RECORD_PATH "record"

#define PATH_LEN 94	// 最好是16的倍数减2，方便16进制工具查看

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

typedef struct {
	char path[PATH_LEN];
	uint32_t code;
} book_t;

#define HASH_GROUP_CNT 3	// 如果为1，就是单链表

int add_book(int book_code, const char* book_path);
int del_book(int book_code);
void print_books();

#endif
