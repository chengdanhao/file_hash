#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>

#define safe_free(p) do { if (p) { free(p); p = NULL; } } while(0)

typedef enum {
	TRAVERSE_ACTION_DO_NOTHING,
	TRAVERSE_ACTION_UPDATE,
	TRAVERSE_ACTION_BREAK,
} traverse_action_t;

typedef enum {
	WITH_PRINT,
	WITHOUT_PRINT,
} print_t;

typedef enum {
	TRAVERSE_ALL,
	TRAVERSE_SPECIFIC_HASH_KEY,
} traverse_type_t;

typedef struct {
	uint32_t hash_key;	// 该字段不能删！！！
	void* hash_value;
} node_data_t;

typedef struct {
	uint8_t used;
	uint32_t next_offset;
	node_data_t data;
} file_node_t;

// 该结构体不能删除，否则会出错，用于记录一些信息
typedef struct {
	void* prop;
} hash_property_t;

int _build_hash_file(const char* f, char* path, uint8_t rebuild);
int get_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));
int set_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));
int add_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*));
int del_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*));
off_t traverse_nodes(char* path, traverse_type_t traverse_type, int hash_key, print_t print, node_data_t* input, traverse_action_t (*cb)(file_node_t*, node_data_t*));
void init_hash_engine(int hash_slot_cnt, int hash_value_size, int hash_property_size);

#define check_hash_file(path)       _build_hash_file(__func__, path, 0)
#define rebuild_hash_file(path)     _build_hash_file(__func__, path, 1)

#endif
