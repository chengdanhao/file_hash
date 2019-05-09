#ifndef __TO_BE_DOWNLOAD_NODE_H__
#define __TO_BE_DOWNLOAD_NODE_H__

#include <stdint.h>
#include "hash.h"

#define TBD_LIST_PATH "to_be_download_list"
#define MAX_TBD_URL_LEN 200
#define MAX_TBD_PATH_LEN 100

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，哈希层代码不用修改
 */

#define TBD_LIST_SLOT_CNT 1

typedef struct {
	char url[MAX_TBD_URL_LEN];
	char path[MAX_TBD_PATH_LEN];
} tbd_node_data_value_t;

typedef struct {
	uint32_t tbd_cnt;
	off_t prev_offset;
	off_t next_offset;
} tbd_header_data_value_t;

int get_tbd(uint32_t key, char url[], char path[]);
int add_tbd(uint32_t key, char url[], char path[]);
void show_tbd_list();
int init_tbd_hash_engine();

#endif

