#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tbd_node.h"
#if 0
#define TBD_INFO 1
#define TBD_DBUG 1
#define TBD_WARN 1
#define TBD_EROR 1

#if TBD_INFO
#define tbd_info(fmt, ...) printf("\e[0;32m[TBD_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define tbd_info(fmt, ...)
#endif

#if TBD_DBUG
#define tbd_debug(fmt, ...) printf("\e[0m[TBD_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define tbd_debug(fmt, ...)
#endif

#if TBD_WARN
#define tbd_warn(fmt, ...) printf("\e[0;33m[TBD_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define tbd_warn(fmt, ...)
#endif

#if TBD_EROR
#define tbd_error(fmt, ...) printf("\e[0;31m[TBD_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define tbd_error(fmt, ...)
#endif

int _get_tbd_list_header_cb(hash_header_data_t* file_header_data, hash_header_data_t* output_header_data) {
	memcpy(output_header_data->value, file_header_data->value, sizeof(tbd_header_data_value_t));

	return 0;
}

int _set_tbd_list_header_cb(hash_header_data_t* file_header_data, hash_header_data_t* input_header_data) {
	memcpy(file_header_data->value, input_header_data->value, sizeof(tbd_header_data_value_t));

	return 0;
}

int _get_tbd_cb(hash_node_t* file_node, hash_node_t* output_node) {
	void *addr = NULL;

	addr = output_node->data.value;
	memcpy(output_node, file_node, sizeof(hash_node_t));

	output_node->data.value = addr;
	memcpy(output_node->data.value, file_node->data.value, sizeof(tbd_node_data_value_t));

	return 0;
}

int _set_tbd_cb(hash_node_t* file_node, hash_node_t* input_node) {
	memcpy(file_node, input_node, sizeof(hash_node_t));
	memcpy(file_node->data.value, input_node->data.value, sizeof(tbd_node_data_value_t));

	return 0;
}

traverse_action_t _find_tbd_cb(hash_node_t* file_node, hash_node_data_t* input_node) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	tbd_node_data_value_t* file_tbd_node_data_value = (tbd_node_data_value_t*)(file_node->data.value);
	tbd_node_data_value_t* input_tbd_node_data_value = (tbd_node_data_value_t*)(input_node->value);

	if (file_node->used && 0 == strcmp(file_tbd_node_data_value->url, input_tbd_node_data_value->url)) {
		memcpy(input_tbd_node_data_value, file_tbd_node_data_value, sizeof(tbd_node_data_value_t));
		action = TRAVERSE_ACTION_BREAK;
	}

	return action;
}

int _add_tbd_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	file_node_data->key = input_node_data->key;
	memcpy(file_node_data->value, input_node_data->value, sizeof(tbd_node_data_value_t));
	return 0;
}

traverse_action_t _print_tbd_node_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	tbd_node_data_value_t* tbd_node_data_value = (tbd_node_data_value_t*)(file_node->data.value);

	if (file_node->used) {
		printf("\e[7;37m%s : %s\e[0m", tbd_node_data_value->url, tbd_node_data_value->path);
	} else {
		printf("*");
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

void get_tbd_list_header(const char* func, const int line, const char* tbd_path,
		tbd_header_data_value_t* tbd_header_data_value) {
	hash_header_data_t hash_header_data;

	memset(&hash_header_data, 0, sizeof(hash_header_data_t));

	hash_header_data.value = tbd_header_data_value;

	get_header(tbd_path, &hash_header_data, _get_tbd_list_header_cb);

#if 0
	printf("<GET> (%s : %d calls) saved_offset = 0x%lX, tbd_cnt = %d.\n",
		func, line,
		tbd_header_data_value->next_offset, tbd_header_data_value->tbd_cnt);
#endif
}

void set_tbd_list_header(const char* func, const int line, const char* tbd_path,
		tbd_header_data_value_t* tbd_header_data_value) {
	hash_header_data_t hash_header_data;

#if 0
	printf("<SET> (%s : %d calls) saved_offset = 0x%lX, tbd_cnt = %d.\n",
		func, line,
		tbd_header_data_value->next_offset, tbd_header_data_value->tbd_cnt);
#endif

	memset(&hash_header_data, 0, sizeof(hash_header_data_t));

	hash_header_data.value = tbd_header_data_value;

	set_header(tbd_path, &hash_header_data, _set_tbd_list_header_cb);
}

// 返回0代表链表中还有元素待处理
int get_tbd(uint32_t key, char url[], char path[]) {
	int ret = -1;
	off_t offset = 0;
	uint32_t tbd_cnt = 0;
	get_node_method_t method = GET_NODE_BY_HASH_SLOT;
	tbd_header_data_value_t tbd_header_data_value;
	hash_node_t node;
	tbd_node_data_value_t tbd_node_data_value;
	
	memset(&tbd_header_data_value, 0, sizeof(tbd_header_data_value));
	memset(&node, 0, sizeof(node));
	memset(&tbd_node_data_value, 0, sizeof(tbd_node_data_value));

	node.data.value = &tbd_node_data_value;

	get_tbd_list_header(__func__, __LINE__, TBD_LIST_PATH, &tbd_header_data_value);

	tbd_cnt = tbd_header_data_value.tbd_cnt;

	if (0 == tbd_cnt) {
		tbd_info("下载列表为空.");
		goto exit;
	}

next_node:
	offset = tbd_header_data_value.next_offset;
	method = offset > 0 ? GET_NODE_BY_OFFSET : GET_NODE_BY_HASH_SLOT;

	get_node(TBD_LIST_PATH, method, key, offset, &node, _get_tbd_cb);

	tbd_header_data_value.next_offset = node.physic_next_offset;
	tbd_header_data_value.prev_offset = node.physic_prev_offset;

	strncpy(url, tbd_node_data_value.url, sizeof(tbd_node_data_value.url));
	strncpy(path, tbd_node_data_value.path, sizeof(tbd_node_data_value.path));

	if (0 == strcmp(tbd_node_data_value.path, "")) {
		goto next_node;
	}

	tbd_info("cnt = %d, url = %s, path = %s.", tbd_cnt, tbd_node_data_value.url, tbd_node_data_value.path);

	--tbd_header_data_value.tbd_cnt;
	set_tbd_list_header(__func__, __LINE__, TBD_LIST_PATH, &tbd_header_data_value);

	ret = 0;

exit:
	return ret;
}

// 如果返回值大于0，说明找到了节点，再取tbd_path
int find_tbd(uint32_t key, char url[]) {
	hash_node_data_t node_data;
	tbd_node_data_value_t tbd_node_data_value;
	int ret = -1;

	memset(&node_data, 0, sizeof(node_data));
	memset(&tbd_node_data_value, 0, sizeof(tbd_node_data_value));

	// 设定一个值用于搜索
	strncpy(tbd_node_data_value.url, url, MAX_TBD_URL_LEN);

	node_data.key = key;
	node_data.value = &tbd_node_data_value;

	if ((ret = traverse_nodes(TBD_LIST_PATH,
					TRAVERSE_SPECIFIC_HASH_SLOT,
					key, WITHOUT_PRINT,
					&node_data, _find_tbd_cb)) > 0) {
		tbd_info("found '%s''", url);
	}

	return ret;
}

int add_tbd(uint32_t key, char url[], char path[]) {
	int ret = -1;
	tbd_header_data_value_t tbd_header_data_value;
	hash_node_data_t node_data;
	tbd_node_data_value_t tbd_node_data_value;

	memset(&tbd_header_data_value, 0, sizeof(tbd_header_data_value));
	memset(&node_data, 0, sizeof(node_data));
	memset(&tbd_node_data_value, 0, sizeof(tbd_node_data_value));

	if (find_tbd(key, url) > 0) {
		tbd_debug("already exist '%s'", url);
		ret = 0;
		goto exit;
	}

	strncpy(tbd_node_data_value.url, url, sizeof(tbd_node_data_value.url));
	strncpy(tbd_node_data_value.path, path, sizeof(tbd_node_data_value.path));

	node_data.key = key;
	node_data.value = &tbd_node_data_value;

	if (0 != (ret = insert_node(TBD_LIST_PATH, &node_data, _add_tbd_cb))) {
		tbd_error("add failed : %s.", url);
		goto exit;
	}

	tbd_info("add success : %s, path = %s.", url, tbd_node_data_value.path);

	get_tbd_list_header(__func__, __LINE__, TBD_LIST_PATH, &tbd_header_data_value);
	++tbd_header_data_value.tbd_cnt;
	set_tbd_list_header(__func__, __LINE__, TBD_LIST_PATH, &tbd_header_data_value);

exit:
	return ret;
}

void show_tbd_list() {
	traverse_nodes(TBD_LIST_PATH, TRAVERSE_ALL, 0, WITH_PRINT, NULL, _print_tbd_node_cb);
}

int init_tbd_hash_engine() {
	return init_hash_engine(TBD_LIST_PATH, FORCE_INIT,
			TBD_LIST_SLOT_CNT, sizeof(tbd_node_data_value_t), sizeof(tbd_header_data_value_t));
}
#endif
