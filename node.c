#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"

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

	if (input_data->book.code == record_data->book.code) {
		node_warn("<success> delete book %d (%s).", input_data->book.code, record_data->book.path); 
		record_data->hash_key = 0;
		record_data->book.code = 0;
		memset(&(record_data->book.path), 0, sizeof(record_data->book.path));
		ret = 0;
	}

	return ret;
}

// print_node接口会释放cb中申请的内存
char* print_node_cb(node_data_t* record_data) {
	char* res = NULL;
	int malloc_size = sizeof(node_data_t) + 20;

	if (NULL == (res = (char*)calloc(1, malloc_size))) {
		node_error("malloc failed.");
		goto exit;
	}

	snprintf(res, malloc_size, "{%d : '%s'}", record_data->book.code, record_data->book.path);

exit:
	return res;
}

int add_book(int book_code, const char* book_path) {
	node_data_t data;
	data.hash_key = data.book.code = book_code;
	strncpy(data.book.path, book_path, sizeof(data.book.path));
	return add_node(RECORD_PATH, &data, add_node_cb);
}

int del_book(int book_code) {
	node_data_t data;
	data.hash_key = data.book.code = book_code;
	return del_node(RECORD_PATH, &data, del_node_cb);
}

void print_books() {
	print_nodes(RECORD_PATH, print_node_cb);
}

