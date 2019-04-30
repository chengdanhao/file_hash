#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>

#define safe_free(p) do { if (p) { free(p); p = NULL; } } while(0)

typedef struct {
	uint32_t hash_key;	// 该字段不能删！！！
	void* hash_value;
} node_data_t;

typedef struct {
	uint8_t used;
	uint32_t next_offset;
	node_data_t data;
} file_node_t;

// 该结构体不能删除，否则会出错
// 用于记录一些信息，暂时没想好放什么
typedef struct {
	void* prop;
} hash_property_t;

int get_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));
int set_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));
int add_node(char* record_path, node_data_t* data, int (*cb)(node_data_t*, node_data_t*));
int del_node(char* record_path, node_data_t* data, int (*cb)(node_data_t*, node_data_t*));
void traverse_nodes(char* path, int (*cb)(file_node_t*));
void init_hash_engine(int hash_slot_cnt, int hash_value_size, int hash_property_size);

#endif
