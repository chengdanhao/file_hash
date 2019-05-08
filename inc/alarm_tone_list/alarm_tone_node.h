#ifndef __ALARM_TONE_H__
#define __ALARM_TONE_H__

#include <stdint.h>
#include "hash.h"

#define ALARM_TONE_LIST_PATH "alarm_tone_list"
#define MAX_ALARM_TONE_PATH_LEN 200

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，哈希层代码不用修改
 */

typedef struct {
	uint32_t time_stamp;
	char path[MAX_ALARM_TONE_PATH_LEN];
} alarm_tone_value_t;

int find_alarm_tone(uint32_t hash_key, uint32_t time_stamp, char* alarm_tone_path);
int add_alarm_tone(uint32_t hash_key, uint32_t time_stamp, char* alarm_tone_path);
int del_alarm_tone(uint32_t hash_key, uint32_t time_stamp);
void show_alarm_tone_list();

#endif

