#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
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

int add_node_cb(node_data_t* file_data, node_data_t* input_data) {
	file_data->hash_key = input_data->hash_key;
	memcpy(file_data->hash_value, input_data->hash_value, sizeof(book_t));
	//node_error(">>>> %s , %d.", ((book_t*)(input_data->hash_value))->path, ((book_t*)(input_data->hash_value))->code);
	return 0;
}

int del_node_cb(node_data_t* file_data, node_data_t* input_data) {
	int ret = -1;
	book_t* file_book = NULL;
	book_t* input_book = NULL;

	file_book = (book_t*)(file_data->hash_value);
	input_book = (book_t*)(input_data->hash_value);

	if (file_book->code == input_book->code) {
		node_warn("<success> delete book %d (%s).", file_book->code, file_book->path); 
		file_data->hash_key = 0;
		file_book->code = 0;
		memset(file_book->path, 0, PATH_LEN);
		ret = 0;
	}

	return ret;
}

char* print_node_cb(node_data_t* file_data) {
	char* res = NULL;
	int malloc_size = sizeof(node_data_t) + 20;
	book_t* file_book = NULL;

	file_book = (book_t*)(file_data->hash_value);

	// print_node接口会释放cb中申请的内存
	if (NULL == (res = (char*)calloc(1, malloc_size))) {
		node_error("malloc failed.");
		goto exit;
	}

	snprintf(res, malloc_size, "{%d : '%s'}", file_book->code, file_book->path);

exit:
	return res;
}

int add_book(int book_code, const char* book_path) {
	int ret = -1;
	node_data_t data;
	book_t book;

	memset(&data, 0, sizeof(data));
	memset(&book, 0, sizeof(book));

	book.code = book_code;
	strncpy(book.path, book_path, sizeof(book.path));

	data.hash_key = book_code;
	data.hash_value = &book;
	ret = add_node(RECORD_PATH, &data, add_node_cb);

exit:
	return ret;
}

int del_book(int book_code) {
	node_data_t data;
	book_t book;

	memset(&data, 0, sizeof(data));
	memset(&book, 0, sizeof(book));

	book.code = book_code;

	data.hash_key = book_code;
	data.hash_value = &book;

	return del_node(RECORD_PATH, &data, del_node_cb);
}

void print_books() {
	print_nodes(RECORD_PATH, print_node_cb);
}

