#include <stdio.h>
#include <string.h>
#include "music_node.h"

void build_story_favorite_playlist() {
	const char* playlist_1[] = {
		"AAA",
		"BBB",
		"CCC",
		"DDD",
		"EEE",
		"FFF",
		"GGG",
		"HHH",
		"III",
		"JJJ",
		"KKK",
	};

	
	const char* del_playlist_1[] = {
		"GGG",
		"HHH",
		"AAA",
		"CCC",
		"KKK",
		"JJJ",
		"III",
#if 0
		"EEE",
		"FFF",
		"BBB",
		"DDD",
#endif
	};

	const char* playlist_2[] = {
		"AAA",
		"BBB",
		"EEE",
		"JJJ",
		"XXX new",
		"YYY new",
		"ZZZ new",
	};

	const char* playlist_3[] = {
		"111",
		"222",
		"333",
		"444",
	};

	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;
	playlist_header_data_value_t playlist_header;

	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));
	memset(&playlist_header, 0, sizeof(playlist_header));

	init_story_playlist_hash_engine();

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(playlist_1[i][0], &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次添加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	for (int i = 0; i < sizeof(del_playlist_1) / sizeof(char*); i++) {
		del_story_music(del_playlist_1[i][0], del_playlist_1[i]);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次删除歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	// 链表为彻底清空，在其中插入
	strncpy(prev_music_data_value.path, "DDD", sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(playlist_2[i][0], &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 2 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	// 以清空方式更新一个链表
	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_3) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_3[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(playlist_3[i][0], &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 3 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");


	for (int j = 0; j < 5; j++) {
		get_story_next_music(0);
	}
	printf("-------------\n");

	for (int j = 0; j < 5; j++) {
		get_story_prev_music(0);
	}
	printf("---------------------------------------\n");
}

void build_album_favorite_playlist() {
	const char* playlist_1_0[] = {
		"0A",
		"0B",
		"0C",
		"0D",
	};
	
	const char* playlist_1_1[] = {
		"1E",
		"1F",
		"1G",
		"1H",
	};
	
	const char* playlist_1_2[] = {
		"2I",
		"2J",
		"2K",
		"2L",
	};
	
	const char* del_playlist_1[] = {
		"0A",
		"1F",
		"0B",
		"2L",
		"2I",
		"1G",		
		"0D",
#if 0
		"2J",
		"1E",
		"0C",
		"1H",
		"2K",
#endif
	};
	
	const char* playlist_2_0[] = {
		"3M",
		"3N",
		"3O",
		"3P",
	};
	
	const char* playlist_2_1[] = {
		"4Q",
		"4R",
		"4S",
		"4T",
	};
	
	const char* playlist_2_2[] = {
		"5U",
		"5V",
		"5W",
		"5X",
	};

	playlist_header_data_value_t header_data_value;
	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;

	memset(&header_data_value, 0, sizeof(header_data_value));
	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));

	init_album_playlist_hash_engine();


	// 开始第一次添加歌曲
	get_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_0[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_1[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_2[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	printf("-- 第 1 次添加歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");


	// 开始第一次删除歌曲
	for (int i = 0; i < sizeof(del_playlist_1) / sizeof(char*); i++) {
		del_album_music(del_playlist_1[i][0], del_playlist_1[i]);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次删除歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	// 开始第二次增加歌曲
	get_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_2_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_0[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, "1H", sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_2_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_1[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, "2J", sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_2_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_2[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_album_music(header_data_value.which_playlist_to_handle,
			&prev_music_data_value,
			&curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	++header_data_value.which_playlist_to_handle;
	set_album_playlist_header(&header_data_value);

	printf("-- 第 2 次增加歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");
#if 0
	// 打印
	for (int i = 0; i < header_data_value.playlist_cnt; ++i) {
		printf("album[%d] -------------\n", i);
		for (int j = 0; j < 5; ++j) {
			get_album_next_music(0);
		}
	}
	printf("\n-------------\n\n");

	for (int i = 0; i < header_data_value.playlist_cnt; ++i) {
		printf("album[%d] -------------\n", i);
		for (int j = 0; j < 5; j++) {
			get_album_next_music(i);
		}
	}
	printf("---------------------------------------\n");
#endif
}

int test_music_playlist_main() {
	build_story_favorite_playlist();
	//build_album_favorite_playlist();

	return 0;
}
