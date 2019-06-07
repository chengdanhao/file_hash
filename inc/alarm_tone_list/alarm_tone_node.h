#ifndef __ALARM_TONE_NODE_H__
#define __ALARM_TONE_NODE_H__

#include <stdint.h>
#include "hash.h"

#define ALARM_TONE_LIST_PATH "alarm_tone_list"
#define MAX_ALARM_TONE_PATH_LEN 200

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，哈希层代码不用修改
 */

#define ALARM_TONE_LIST_SLOT_CNT 1

typedef struct {
	uint32_t time_stamp;
	char path[MAX_ALARM_TONE_PATH_LEN];
} alarm_tone_data_value_t;

int find_alarm_tone(uint32_t time_stamp);
int insert_alarm_tone(const alarm_tone_data_value_t* prev_alarm_tone_data_value,
		const alarm_tone_data_value_t* curr_alarm_tone_data_value);
int del_alarm_tone(uint32_t time_stamp);
void clean_alarm_tone_list();
void show_alarm_tone_list();
int init_alarm_tone_hash_engine();

#endif

