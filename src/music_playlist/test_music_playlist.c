#include <stdio.h>
#include <string.h>
#include "music_node.h"

void diff_story_playlist() {
#define TEST_CASE 3
	const char* playlist_1[] = {
		"AAA",
		"BBB",
		"CCC",
		"DDD",
	};

#if (TEST_CASE == 1)
	const char* playlist_2[] = {
		"CCC",
		"DDD",
		"EEE",
		"FFF",
	};
#elif (TEST_CASE == 2)
	const char* playlist_2[] = {
		"EEE",
		"FFF",
	};
#elif (TEST_CASE == 3)
	const char* playlist_2[] = {
		"BBB",
		"EEE",
		"FFF",
	};
#endif

	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;

	memset(&prev_music_data_value, 0, sizeof(music_data_value_t));
	memset(&curr_music_data_value, 0, sizeof(music_data_value_t));

	init_story_playlist_hash_engine();

	// 普通添加时 curr_music 的 delete_or_not 标记是 MUSIC_KEEP
	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		add_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

#if 0
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		del_story_music(playlist_1[i]);
	}
#endif

	printf("-- 原始故事列表 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	// 里面会把将节点的 delete_or_not 标记是 MUSIC_TO_BE_DELETE
	pre_diff_story_playlist();

	/* 将新列表中的歌插入老链表头部，并将节点delete_or_not标记设置为MUSIC_TO_BE_DOWNLOAD。
	 * 在add_music时，如果已存在的节点，会将delete_or_not标记设置为MUSIC_KEEP
	 * 因此循环体执行完后，MUSIC_TO_BE_DELETE, MUSIC_TO_BE_DOWNLOAD, MUSIC_KEEP
	 * 都会在分别标记出来
	 */
	get_story_first_node(&prev_music_data_value);
	for (int i = 0; i < sizeof(playlist_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_TO_BE_DOWNLOAD;
		strncpy(curr_music_data_value.path, playlist_2[i], sizeof(curr_music_data_value.path));
		add_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- diff ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");


	init_story_download_list_hash_engine();
	init_story_delete_list_hash_engine();

	post_diff_story_playlist();

	printf("-- post-diff ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	show_story_download_list();
	show_story_delete_list();
}

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

	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));

	init_story_playlist_hash_engine();

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		//printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次添加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	for (int i = 0; i < sizeof(del_playlist_1) / sizeof(char*); i++) {
		del_story_music(del_playlist_1[i]);
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
		add_story_music(&prev_music_data_value, &curr_music_data_value);
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
		add_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 3 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");


	for (int j = 0; j < 5; j++) {
		get_story_next_music();
	}
	printf("-------------\n");

	for (int j = 0; j < 5; j++) {
		get_story_prev_music();
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
	diff_story_playlist();
	//build_story_favorite_playlist();
	//build_album_favorite_playlist();

	return 0;
}
