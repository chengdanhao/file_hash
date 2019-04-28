#include <stdio.h>
#include <string.h>
#include "node.h"

#define NODE_INFO 1
#define NODE_DBUG 1
#define NODE_WARN 1
#define NODE_EROR 1

#if NODE_INFO
#define node_info(fmt, ...) printf("\e[0;32m[NODE_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_info(fmt, ...)
#endif

#if NODE_DBUG
#define node_debug(fmt, ...) printf("\e[0m[NODE_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_debug(fmt, ...)
#endif

#if NODE_WARN
#define node_warn(fmt, ...) printf("\e[0;33m[NODE_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_warn(fmt, ...)
#endif

#if NODE_EROR
#define node_error(fmt, ...) printf("\e[0;31m[NODE_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_error(fmt, ...)
#endif

int add_node_cb(node_data_t* record_data, node_data_t* input_data) {
	memcpy(record_data, input_data, sizeof(node_data_t));
	return 0;
}

int del_node_cb(node_data_t* record_data, node_data_t* input_data) {
	int ret = -1;

	if (input_data->book_code == record_data->book_code) {
		node_warn("<success> delete book %d (%s).", input_data->book_code, record_data->path); 
		record_data->hash_key = 0;
		record_data->book_code = 0;
		memset(&(record_data->path), 0, sizeof(record_data->path));
		ret = 0;
	}

	return ret;
}

