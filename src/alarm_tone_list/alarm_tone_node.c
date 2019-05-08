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

int _add_alarm_tone_cb(node_data_t* file, node_data_t* input) {
	file->key = input->key;
	memcpy(file->value, input->value, sizeof(alarm_tone_value_t));
	return 0;
}

int _del_alarm_tone_cb(node_data_t* file, node_data_t* input) {
	int ret = -1;
	alarm_tone_value_t* file_alarm_tone = (alarm_tone_value_t*)(file->value);
	alarm_tone_value_t* input_alarm_tone = (alarm_tone_value_t*)(input->value);

	if (file_alarm_tone->time_stamp == input_alarm_tone->time_stamp) {
		file->key = 0;
		file_alarm_tone->time_stamp = 0;
		memset(file_alarm_tone->path, 0, MAX_ALARM_TONE_PATH_LEN);
		ret = 0;
	}

	return ret;
}

traverse_action_t _print_alarm_tone_node_cb(file_node_t* node, node_data_t* input) {
	alarm_tone_value_t* alarm_tone = (alarm_tone_value_t*)(node->data.value);

	if (node->used) {
		printf("\e[7;37m%u : %s\e[0m", alarm_tone->time_stamp, alarm_tone->path);
	} else {
		printf("*");
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t _find_alarm_tone_cb(file_node_t* node, node_data_t* input) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	alarm_tone_value_t* file_alarm_tone = (alarm_tone_value_t*)(node->data.value);
	alarm_tone_value_t* input_alarm_tone = (alarm_tone_value_t*)(input->value);

	if (node->used && file_alarm_tone->time_stamp == input_alarm_tone->time_stamp) {
		memcpy(input_alarm_tone, file_alarm_tone, sizeof(alarm_tone_value_t));
		action = TRAVERSE_ACTION_BREAK;
	}

	return action;
}

// 如果返回值大于0，说明找到了节点，再取alarm_tone_path
int find_alarm_tone(uint32_t key, uint32_t time_stamp, char* alarm_tone_path) {
	node_data_t data;
	alarm_tone_value_t alarm_tone;
	int ret = -1;

	memset(&data, 0, sizeof(data));
	memset(&alarm_tone, 0, sizeof(alarm_tone));

	alarm_tone.time_stamp = time_stamp;

	data.key = key;
	data.value = &alarm_tone;

	if ((ret = traverse_nodes(ALARM_TONE_LIST_PATH,
			TRAVERSE_SPECIFIC_HASH_KEY,
			key, WITHOUT_PRINT,
			&data, _find_alarm_tone_cb)) > 0) {
		strncpy(alarm_tone_path, alarm_tone.path, MAX_ALARM_TONE_PATH_LEN);
		at_info("found '%d', tone is '%s'", time_stamp, alarm_tone_path);
	}

	return ret;
}

int add_alarm_tone(uint32_t key, uint32_t time_stamp, char* alarm_tone_path) {
	int ret = -1;
	node_data_t data;
	alarm_tone_value_t alarm_tone;

	if (find_alarm_tone(key, time_stamp, alarm_tone_path) > 0) {
		at_debug("already exist '%d'", time_stamp);
		ret = 0;
		goto exit;
	}

	memset(&data, 0, sizeof(data));
	memset(&alarm_tone, 0, sizeof(alarm_tone));

	alarm_tone.time_stamp = time_stamp;
	strncpy(alarm_tone.path, alarm_tone_path, sizeof(alarm_tone.path));

	data.key = key;
	data.value = &alarm_tone;

	if (0 != (ret = add_node(ALARM_TONE_LIST_PATH, &data, _add_alarm_tone_cb))) {
		at_error("add failed : %s.", alarm_tone_path);
		goto exit;
	}

	at_info("add success : %s.", alarm_tone_path);

exit:
	return ret;
}

int del_alarm_tone(uint32_t key, uint32_t time_stamp) {
	int ret = -1;
	node_data_t data;
	alarm_tone_value_t alarm_tone;

	memset(&data, 0, sizeof(data));
	memset(&alarm_tone, 0, sizeof(alarm_tone));

	alarm_tone.time_stamp = time_stamp;

	data.key = key;
	data.value = &alarm_tone;

	if (0 != (ret = del_node(ALARM_TONE_LIST_PATH, &data, _del_alarm_tone_cb))) {
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
		ALARM_TONE_LIST_SLOT_CNT,sizeof(alarm_tone_value_t), 0);
}
