#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>

#define safe_free(p) do { if (p) { free(p); p = NULL; } } while(0)

// 遍历拿到所需数据后采取的动作，可以通过 “|” 的方式叠加动作
typedef enum {
	TRAVERSE_ACTION_DO_NOTHING,
	TRAVERSE_ACTION_UPDATE,
	TRAVERSE_ACTION_BREAK,
} traverse_action_t;

// 遍历节点时是否打印相关信息
typedef enum {
	WITH_PRINT,
	WITHOUT_PRINT,
} print_t;

// 遍历所有节点，还是指定哈希槽的节点
typedef enum {
	TRAVERSE_ALL,
	TRAVERSE_SPECIFIC_HASH_KEY,
} traverse_type_t;

// 获取节点的方式，通过哈希值取得“首节点”还是通过偏移量直接定位
typedef enum {
	GET_NODE_BY_HASH_KEY,
	GET_NODE_BY_OFFSET,
} get_node_method_t;

// 节点的数据部分，分key和value，其中value由用户再上层填充
typedef struct {
	uint32_t hash_key;	// 该字段不能删！！！
	void* hash_value;
} node_data_t;

// 文件节点，以链式方式存储在文件中
typedef struct {
	uint8_t used;
	off_t prev_offset;
	off_t next_offset;
	node_data_t data;
} file_node_t;

// 记录哈希链表的一些属性，由上层填充
// 该结构体不能删除
typedef struct {
	void* prop;
} hash_property_t;

// 创建哈希文件，一般不会直接调用。
int _build_hash_file(const char* f, char* path, uint8_t rebuild);

// 获取哈希属性
int get_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));

// 设置哈希属性
int set_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*));

// 获取节点信息
off_t get_node(char* path, get_node_method_t method, uint32_t hash_key, off_t offset, file_node_t* output, int (*cb)(file_node_t*, file_node_t*));

// 添加节点
int add_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*));

// 删除节点
int del_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*));

// 遍历节点
uint8_t traverse_nodes(char* path, traverse_type_t traverse_type, uint32_t hash_key, print_t print, node_data_t* input, traverse_action_t (*cb)(file_node_t*, node_data_t*));

// 初始化哈希引擎，告知所需信息
void init_hash_engine(int hash_slot_cnt, int hash_value_size, int hash_property_size);

// 调用下面两个函数“创建”或“重建”哈希文件
#define check_hash_file(path)       _build_hash_file(__func__, path, 0)
#define rebuild_hash_file(path)     _build_hash_file(__func__, path, 1)

#endif
