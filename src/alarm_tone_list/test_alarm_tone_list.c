#include <stdio.h>
#include <string.h>
#include "alarm_tone_node.h"

void build_alarm_tone_list() {
	alarm_tone_data_value_t playlist_add_1[] = {
		{ 1, "A" },
		{ 2, "B" },
		{ 3, "C" },
		{ 4, "D" },
		{ 5, "E" },
	};

	alarm_tone_data_value_t playlist_del_1[] = {
		{ 1, "A" },
		{ 3, "C" },
		{ 5, "E" },
	};

	alarm_tone_data_value_t playlist_add_2[] = {
		{ 2, "B" },
		{ 6, "F" },
		{ 7, "G" },
		{ 4, "D" },
		{ 8, "H" },
	};
	
	alarm_tone_data_value_t prev_alarm_tone_data_value;
	alarm_tone_data_value_t curr_alarm_tone_data_value;

	memset(&prev_alarm_tone_data_value, 0, sizeof(alarm_tone_data_value_t));
	memset(&curr_alarm_tone_data_value, 0, sizeof(alarm_tone_data_value_t));

	// 1. 初始化哈希引擎，不需要哈希头部，所以写0
	init_alarm_tone_hash_engine();

	// 3. 初始化播放列表
	for (int i = 0; i < sizeof(playlist_add_1) / sizeof(playlist_add_1[0]); i++) {
		curr_alarm_tone_data_value.time_stamp = playlist_add_1[i].time_stamp;
		strncpy(curr_alarm_tone_data_value.path, playlist_add_1[i].path, sizeof(curr_alarm_tone_data_value.path));
		insert_alarm_tone(&prev_alarm_tone_data_value, &curr_alarm_tone_data_value);
		prev_alarm_tone_data_value = curr_alarm_tone_data_value;
	}

	show_alarm_tone_list();

	for (int i = 0; i < sizeof(playlist_del_1) / sizeof(playlist_del_1[0]); i++) {
		del_alarm_tone(playlist_del_1[i].time_stamp);
	}

	show_alarm_tone_list();

	for (int i = 0; i < sizeof(playlist_add_2) / sizeof(playlist_add_2[0]); i++) {
		curr_alarm_tone_data_value.time_stamp = playlist_add_2[i].time_stamp;
		strncpy(curr_alarm_tone_data_value.path, playlist_add_2[i].path, sizeof(curr_alarm_tone_data_value.path));
		insert_alarm_tone(&prev_alarm_tone_data_value, &curr_alarm_tone_data_value);
		prev_alarm_tone_data_value = curr_alarm_tone_data_value;
	}

	show_alarm_tone_list();

	find_alarm_tone(1);
	find_alarm_tone(2);
	find_alarm_tone(3);
	find_alarm_tone(6);
	find_alarm_tone(7);
}

int test_alarm_tone_list_main() {
	build_alarm_tone_list();
	return 0;
}
