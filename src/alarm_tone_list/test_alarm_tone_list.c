#include <stdio.h>
#include <string.h>
#include "alarm_tone_node.h"

void build_alarm_tone_list() {
	alarm_tone_value_t playlist_add_1[] = {
		{ 1, "A" },
		{ 2, "B" },
		{ 3, "C" },
		{ 4, "D" },
		{ 5, "E" },
	};

	alarm_tone_value_t playlist_del_1[] = {
		{ 1, "A" },
		{ 3, "C" },
		{ 5, "E" },
	};

	alarm_tone_value_t playlist_add_2[] = {
		{ 2, "B" },
		{ 4, "D" },
		{ 6, "F" },
		{ 7, "G" },
		{ 8, "H" },
	};

	uint32_t hash_key = 0;
	char tone_path[MAX_ALARM_TONE_PATH_LEN] = {0};

	// 1. 初始化哈希引擎，不需要哈希头部，所以写0
	init_alarm_tone_hash_engine();

	// 3. 初始化播放列表
	for (int i = 0; i < sizeof(playlist_add_1) / sizeof(playlist_add_1[0]); i++) {
		add_alarm_tone(hash_key, playlist_add_1[i].time_stamp, playlist_add_1[i].path);
	}

	show_alarm_tone_list();

	for (int i = 0; i < sizeof(playlist_del_1) / sizeof(playlist_del_1[0]); i++) {
		del_alarm_tone(hash_key, playlist_del_1[i].time_stamp);
	}

	show_alarm_tone_list();

	for (int i = 0; i < sizeof(playlist_add_2) / sizeof(playlist_add_2[0]); i++) {
		add_alarm_tone(hash_key, playlist_add_2[i].time_stamp, playlist_add_2[i].path);
	}

	show_alarm_tone_list();

	find_alarm_tone(hash_key, 1, tone_path);
	find_alarm_tone(hash_key, 2, tone_path);
	find_alarm_tone(hash_key, 3, tone_path);
	find_alarm_tone(hash_key, 6, tone_path);
	find_alarm_tone(hash_key, 7, tone_path);
}

int test_alarm_tone_list_main() {
	build_alarm_tone_list();
	return 0;
}

