#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

#define PATH_LEN 94	// 最好是16的倍数减2，方便16进制工具查看

typedef struct {
	uint32_t hash_key;	// 该字段不能删！！！
	uint32_t book_code;
	char path[PATH_LEN + 1];
} node_data_t;

#define HASH_GROUP_CNT 3	// 如果为1，就是单链表

int add_node_cb(node_data_t* record_data, node_data_t* input_data);
int del_node_cb(node_data_t* record_data, node_data_t* input_data);

#endif
