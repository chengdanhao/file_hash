#ifndef __HASH_H__
#define __HASH_H__

#include <stdint.h>
#include <stdbool.h>

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
} printable_t;

// 遍历所有节点，还是指定哈希槽的节点
typedef enum {
	TRAVERSE_ALL,
	TRAVERSE_SPECIFIC_HASH_SLOT,
} traverse_type_t;

typedef enum {
	TRAVERSE_BY_LOGIC,
	TRAVERSE_BY_PHYSIC,
} traverse_by_what_t;

// 获取节点的方式，通过哈希值取得“首节点”还是通过偏移量直接定位
typedef enum {
	GET_NODE_BY_HASH_SLOT,
	GET_NODE_BY_OFFSET,
} get_node_method_t;

typedef enum {
	GENTLE_INIT,
	FORCE_INIT,
} init_method_t;

typedef struct {
	off_t logic_prev;	// 按序链接后的逻辑顺序
	off_t logic_next;
	off_t physic_prev;	// 物理层存储顺序
	off_t physic_next;
} offset_t;


/************************************************
 * 哈希节点的信息
 ***********************************************/

// 节点的数据部分，value由用户在上层填充，其他字段不能删除！！
typedef struct {
	bool is_first_node;	// 表示是否是逻辑第一个节点
	uint32_t key;	
	void* value;
} hash_node_data_t;

// 文件节点，以链式方式存储在文件中
typedef struct {
	uint8_t used;
	offset_t offsets;		//每个节点的偏移量信息
	hash_node_data_t data;
} hash_node_t;

/*****************************************************/


/************************************************
 * 哈希文件头部信息，记录了当前哈希文件的整体信息
 * 及每个哈希槽的基本信息
 ***********************************************/

typedef struct {
	void* value;
} hash_header_data_t;

// 记录每个哈希槽的信息
typedef struct {
	off_t first_logic_node_offset;	// 记录排序后第一个节点位置
	uint32_t node_cnt;				// 记录每个槽中节点个数
	offset_t offsets;				// 最近一次访问的节点偏移量信息
} slot_info_t;

// 记录哈希链表的一些属性，由上层填充
typedef struct {
	uint32_t slot_cnt;
	uint32_t header_data_value_size;
	uint32_t node_data_value_size;
	slot_info_t *slots;
	hash_header_data_t data;
} hash_header_t;

/*****************************************************/

// 获取哈希属性
int get_header(const char* path, hash_header_data_t* output_header_data,
		int (*cb)(hash_header_data_t*, hash_header_data_t*));

// 设置哈希属性
int set_header(const char* path, hash_header_data_t* iutput_heade_data,
		int (*cb)(hash_header_data_t*, hash_header_data_t*));

// 获取节点信息
int get_node(const char* path, get_node_method_t method, uint32_t hash_key,
		off_t offset, hash_node_t* output_node, int (*cb)(hash_node_t*, hash_node_t*));

int set_node(const char* path, get_node_method_t method, uint32_t hash_key,
		off_t offset, hash_node_t* input_node, int (*cb)(hash_node_t*, hash_node_t*));

// 添加节点
int add_node(const char* path,
		hash_node_data_t* input_prev_node_data, hash_node_data_t* input_curr_node_data,
		int (*cb)(hash_node_data_t*, hash_node_data_t*));

// 删除节点
int del_node(const char* path, hash_node_data_t* input_node_data,
		int (*cb)(hash_node_data_t*, hash_node_data_t*));

// 遍历节点
uint8_t traverse_nodes(const char* path, traverse_type_t traverse_type, traverse_by_what_t by_what,
		uint32_t hash_key, printable_t print, hash_node_data_t* input_node_data,
		traverse_action_t (*cb)(hash_node_t*, hash_node_data_t*));

// 初始化哈希引擎，告知所需信息
int init_hash_engine(const char* path, init_method_t rebuild,
		int hash_slot_cnt, int node_data_value_size, int header_data_value_size);

#endif
