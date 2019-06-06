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

bool _add_alarm_tone_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_prev_node_data) {
	alarm_tone_data_value_t *file_alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node_data->value);
	alarm_tone_data_value_t *input_prev_alarm_tone_data_value = (alarm_tone_data_value_t*)(input_prev_node_data->value);

	return file_alarm_tone_data_value->time_stamp == input_prev_alarm_tone_data_value->time_stamp;
}

bool _del_alarm_tone_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	alarm_tone_data_value_t* file_alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node_data->value);
	alarm_tone_data_value_t* input_alarm_tone_data_value = (alarm_tone_data_value_t*)(input_node_data->value);

	return file_alarm_tone_data_value->time_stamp == input_alarm_tone_data_value->time_stamp;
}

traverse_action_t _print_alarm_tone_node_cb(hash_node_data_t* file_node_data, void* input_arg) {
	alarm_tone_data_value_t* alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node_data->value);

	printf("\e[7;37m%u : %s\e[0m", alarm_tone_data_value->time_stamp, alarm_tone_data_value->path);

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t _find_alarm_tone_cb(hash_node_data_t* file_node_data, void* input_arg) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	alarm_tone_data_value_t* file_alarm_tone_data_value = (alarm_tone_data_value_t*)(file_node_data->value);
	alarm_tone_data_value_t* input_alarm_tone_data_value = (alarm_tone_data_value_t*)input_arg;

	if (file_alarm_tone_data_value->time_stamp == input_alarm_tone_data_value->time_stamp) {
		strncpy(input_alarm_tone_data_value->path, file_alarm_tone_data_value->path, MAX_ALARM_TONE_PATH_LEN);
		action = TRAVERSE_ACTION_BREAK;
	}

	return action;
}

// 如果返回值大于0，说明找到了节点，再取alarm_tone_path
int find_alarm_tone(uint32_t time_stamp) {
	int ret = -1;
	alarm_tone_data_value_t alarm_tone_data_value;

	memset(&alarm_tone_data_value, 0, sizeof(alarm_tone_data_value));

	alarm_tone_data_value.time_stamp = time_stamp;

	if ((ret = traverse_nodes(ALARM_TONE_LIST_PATH,
					TRAVERSE_BY_LOGIC,
					0, WITHOUT_PRINT,
					(void*)&alarm_tone_data_value, _find_alarm_tone_cb)) > 0) {
		at_info("found '%d', tone is '%s'", time_stamp, alarm_tone_data_value.path);
	}

	return ret;
}

int insert_alarm_tone(const alarm_tone_data_value_t* prev_alarm_tone_data_value,
		const alarm_tone_data_value_t* curr_alarm_tone_data_value) {
	int ret = -1;
	hash_node_data_t prev_node_data;
	hash_node_data_t curr_node_data;

	memset(&prev_node_data, 0, sizeof(prev_node_data));
	memset(&curr_node_data, 0, sizeof(curr_node_data));

	prev_node_data.key = 0;
	prev_node_data.value = (void*)prev_alarm_tone_data_value;

	curr_node_data.key = 0;
	curr_node_data.value = (void*)curr_alarm_tone_data_value;

	if (find_alarm_tone(curr_alarm_tone_data_value->time_stamp) > 0) {
		at_debug("already exist '%s'", curr_alarm_tone_data_value->path);
		ret = 0;
		goto exit;
	}

	// 第一个节点
	if (0 == strcmp(DUMMY_ALARM_TONE_PATH, prev_alarm_tone_data_value->path)) {
		at_debug("[ + ] first node '%s' to '%s'.", curr_alarm_tone_data_value->path, ALARM_TONE_LIST_PATH);
		curr_node_data.is_first_node = true;
	}

	if (0 != (ret = insert_node(ALARM_TONE_LIST_PATH, &prev_node_data, &curr_node_data, _add_alarm_tone_cb))) {
		at_error("[ + ] '%s' to '%s' failed!", curr_alarm_tone_data_value->path, ALARM_TONE_LIST_PATH);
		goto exit;
	}

	at_info("[ + ] '%s' to '%s' success.", curr_alarm_tone_data_value->path, ALARM_TONE_LIST_PATH);

exit:
	return ret;
}


int del_alarm_tone(uint32_t time_stamp) {
	int ret = -1;
	hash_node_data_t node_data;
	alarm_tone_data_value_t alarm_tone_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&alarm_tone_data_value, 0, sizeof(alarm_tone_data_value));

	alarm_tone_data_value.time_stamp = time_stamp;

	node_data.key = 0;
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
	traverse_nodes(ALARM_TONE_LIST_PATH, TRAVERSE_BY_LOGIC,
			ALARM_TONE_LIST_SLOT_CNT, WITH_PRINT, NULL, _print_alarm_tone_node_cb);
}

int init_alarm_tone_hash_engine() {
	return init_hash_engine(ALARM_TONE_LIST_PATH, FORCE_INIT,
			ALARM_TONE_LIST_SLOT_CNT, sizeof(alarm_tone_data_value_t), 0);
}
