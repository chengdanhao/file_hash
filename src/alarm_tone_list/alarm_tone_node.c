#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alarm_tone_node.h"

#define ALARM_TONE_INFO 1
#define ALARM_TONE_DBUG 1
#define ALARM_TONE_WARN 1
#define ALARM_TONE_EROR 1

#if ALARM_TONE_INFO
#define at_info(fmt, ...) printf("\e[0;32m[ALARM_TONE_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define at_info(fmt, ...)
#endif

#if ALARM_TONE_DBUG
#define at_debug(fmt, ...) printf("\e[0m[ALARM_TONE_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define at_debug(fmt, ...)
#endif

#if ALARM_TONE_WARN
#define at_warn(fmt, ...) printf("\e[0;33m[ALARM_TONE_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define at_warn(fmt, ...)
#endif

#if ALARM_TONE_EROR
#define at_error(fmt, ...) printf("\e[0;31m[ALARM_TONE_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define at_error(fmt, ...)
#endif

int _add_alarm_tone_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	file_node_data->key = input_node_data->key;
	memcpy(file_node_data->value, input_node_data->value, sizeof(alarm_tone_data_value_t));
	return 0;
}

int _del_alarm_tone_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	int ret = -1;

	alarm_tone_data_value_t* file_alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node_data->value);
	alarm_tone_data_value_t* input_alarm_tone_data_value = (alarm_tone_data_value_t*)(input_node_data->value);

	if (file_alarm_tone_data_value->time_stamp == input_alarm_tone_data_value->time_stamp) {
		file_node_data->key = 0;
		file_alarm_tone_data_value->time_stamp = 0;
		memset(file_alarm_tone_data_value->path, 0, MAX_ALARM_TONE_PATH_LEN);
		ret = 0;
	}

	return ret;
}

traverse_action_t _print_alarm_tone_node_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	alarm_tone_data_value_t* alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node->data.value);

	if (file_node->used) {
		printf("\e[7;37m%u : %s\e[0m", alarm_tone_data_value->time_stamp, alarm_tone_data_value->path);
	} else {
		printf("*");
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t _find_alarm_tone_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	alarm_tone_data_value_t* file_alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node->data.value);
	alarm_tone_data_value_t* input_alarm_tone_data_value = (alarm_tone_data_value_t*)(input_node_data->value);

	if (file_node->used && file_alarm_tone_data_value->time_stamp == input_alarm_tone_data_value->time_stamp) {
		memcpy(input_alarm_tone_data_value, file_alarm_tone_data_value, sizeof(alarm_tone_data_value_t));
		action = TRAVERSE_ACTION_BREAK;
	}

	return action;
}

// 如果返回值大于0，说明找到了节点，再取alarm_tone_path
int find_alarm_tone(uint32_t key, uint32_t time_stamp, char* alarm_tone_path) {
	int ret = -1;
	hash_node_data_t node_data;
	alarm_tone_data_value_t alarm_tone_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&alarm_tone_data_value, 0, sizeof(alarm_tone_data_value));

	alarm_tone_data_value.time_stamp = time_stamp;

	node_data.key = key;
	node_data.value = &alarm_tone_data_value;

	if ((ret = traverse_nodes(ALARM_TONE_LIST_PATH,
					TRAVERSE_SPECIFIC_HASH_SLOT,
					key, WITHOUT_PRINT,
					&node_data, _find_alarm_tone_cb)) > 0) {
		strncpy(alarm_tone_path, alarm_tone_data_value.path, MAX_ALARM_TONE_PATH_LEN);
		at_info("found '%d', tone is '%s'", time_stamp, alarm_tone_path);
	}

	return ret;
}

int add_alarm_tone(uint32_t key, uint32_t time_stamp, char* alarm_tone_path) {
	int ret = -1;
	hash_node_data_t node_data;
	alarm_tone_data_value_t alarm_tone_data_value;

	if (find_alarm_tone(key, time_stamp, alarm_tone_path) > 0) {
		at_debug("already exist '%d'", time_stamp);
		ret = 0;
		goto exit;
	}

	memset(&node_data, 0, sizeof(node_data));
	memset(&alarm_tone_data_value, 0, sizeof(alarm_tone_data_value));

	alarm_tone_data_value.time_stamp = time_stamp;
	strncpy(alarm_tone_data_value.path, alarm_tone_path, sizeof(alarm_tone_data_value.path));

	node_data.key = key;
	node_data.value = &alarm_tone_data_value;

	if (0 != (ret = add_node(ALARM_TONE_LIST_PATH, &node_data, _add_alarm_tone_cb))) {
		at_error("add failed : %s.", alarm_tone_path);
		goto exit;
	}

	at_info("add success : %s.", alarm_tone_path);

exit:
	return ret;
}

int del_alarm_tone(uint32_t key, uint32_t time_stamp) {
	int ret = -1;
	hash_node_data_t node_data;
	alarm_tone_data_value_t alarm_tone_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&alarm_tone_data_value, 0, sizeof(alarm_tone_data_value));

	alarm_tone_data_value.time_stamp = time_stamp;

	node_data.key = key;
	node_data.value = &alarm_tone_data_value;

	if (0 != (ret = del_node(ALARM_TONE_LIST_PATH, &node_data, _del_alarm_tone_cb))) {
		at_error("del failed : %d.", time_stamp);
		goto exit;
	}

	at_warn("del success : %d.", time_stamp);

exit:
	return ret;
}

void show_alarm_tone_list() {
	traverse_nodes(ALARM_TONE_LIST_PATH, TRAVERSE_ALL, 0, WITH_PRINT, NULL, _print_alarm_tone_node_cb);
}

int init_alarm_tone_hash_engine() {
	return init_hash_engine(ALARM_TONE_LIST_PATH, FORCE_INIT,
			ALARM_TONE_LIST_SLOT_CNT, sizeof(alarm_tone_data_value_t), 0);
}
