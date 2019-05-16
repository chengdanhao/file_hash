#include <stdio.h>
#include <string.h>
#include "music_node.h"

// 返回对应的哈希槽
uint32_t find_slot_no_by_chan_name(const char* name) {
	uint32_t slot_no = 0;
	playlist_header_data_value_t header_data_value;

	memset(&header_data_value, 0, sizeof(header_data_value));

	// 开始第一次添加歌曲
	get_album_playlist_header(&header_data_value);

	for (int i = 0; i < header_data_value.playlist_cnt; ++i) {
		if (0 == strncmp(name, header_data_value.playlist[i].name, MAX_PLAYLIST_NAME_LEN)) {
			slot_no = i;
			goto exit;
		}
	}

	slot_no = header_data_value.which_playlist_to_handle++;
	strncpy(header_data_value.playlist[slot_no].name, name, MAX_PLAYLIST_NAME_LEN);
	set_album_playlist_header(&header_data_value);

exit:
	return slot_no;
}

void diff_story_playlist() {
	const char* playlist_1[] = {
		"AAA",
		"BBB",
		"CCC",
		"DDD",
	};

#define STORY_TEST_CASE 3
#if (STORY_TEST_CASE == 1)
	const char* playlist_2[] = {
		"CCC",
		"DDD",
		"EEE",
		"FFF",
	};
#elif (STORY_TEST_CASE == 2)
	const char* playlist_2[] = {
		"EEE",
		"FFF",
	};
#elif (STORY_TEST_CASE == 3)
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
		curr_music_data_value.which_slot = 0;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		insert_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

#if 0
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		delete_story_music(playlist_1[i]);
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
		curr_music_data_value.which_slot = 0;
		insert_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- diff ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	post_diff_story_playlist();

	printf("-- post-diff ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	printf("删除列表 : ");
	show_story_delete_list();
	printf("下载列表 : ");
	show_story_download_list();
}

void diff_album_playlist() {
	const char* channel_1_0 = "chan_1_0";
	const char* playlist_1_0[] = {
		"0A",
		"0B",
		"0C",
		"0D",
	};

	const char* channel_1_1 = "chan_1_1";
	const char* playlist_1_1[] = {
		"1E",
		"1F",
		"1G",
		"1H",
	};

	const char* channel_1_2 = "chan_1_2";
	const char* playlist_1_2[] = {
		"2I",
		"2J",
		"2K",
		"2L",
	};

	const char* channel_2_0 = "chan_1_0";
	const char* playlist_2_0[] = {
		"0A",
		"0B",
		"0D",
		"01_new",
		"02_new",
	};

	const char* channel_2_1 = "chan_1_1";
	const char* playlist_2_1[] = {
		"1F",
		"1H",
		"1R_new",
		"1S_new",
		"1T_new",
	};

	const char* channel_2_2 = "chan_2_2";
	const char* playlist_2_2[] = {
		"2U_new",
		"2V_new",
		"2W_new",
		"2X_new",
	};

	uint32_t which_slot = 0;
	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;

	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));

	init_album_playlist_hash_engine();

	// 开始第一次添加歌曲
	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = 0;
	printf("添加 %s 到 %d 哈希槽\n", channel_1_0, which_slot);
	for (int i = 0; i < sizeof(playlist_1_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_0[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = 1;
	printf("添加 %s 到 %d 哈希槽\n", channel_1_1, which_slot);
	for (int i = 0; i < sizeof(playlist_1_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_1[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = 2;
	printf("添加 %s 到 %d 哈希槽\n", channel_1_2, which_slot);
	for (int i = 0; i < sizeof(playlist_1_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_2[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 原始专辑歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	// 预处理
	pre_diff_album_playlist();

	// 开始第二次增加歌曲
	which_slot = 0;
	get_album_first_node_in_slot(which_slot, &prev_music_data_value);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_0, which_slot);
	for (int i = 0; i < sizeof(playlist_2_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_TO_BE_DOWNLOAD;
		strncpy(curr_music_data_value.path, playlist_2_0[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	which_slot = 1;
	get_album_first_node_in_slot(which_slot, &prev_music_data_value);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_1, which_slot);
	for (int i = 0; i < sizeof(playlist_2_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_TO_BE_DOWNLOAD;
		strncpy(curr_music_data_value.path, playlist_2_1[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	which_slot = 2;
	get_album_first_node_in_slot(which_slot, &prev_music_data_value);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_2, which_slot);
	for (int i = 0; i < sizeof(playlist_2_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_TO_BE_DOWNLOAD;
		strncpy(curr_music_data_value.path, playlist_2_2[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- diff专辑歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	// 处理完了后
	post_diff_album_playlist();

	printf("-- post-diff专辑歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	printf("-- 下载列表 ---------------------------------------\n");
	show_album_download_list();
	printf("----------------------------------------------------------\n");

	printf("-- 删除列表 ---------------------------------------\n");
	show_album_delete_list();
	printf("----------------------------------------------------------\n");
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

	uint32_t music_cnt = 0;
	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;

	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));

	init_story_playlist_hash_engine();

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = 0;
		insert_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次添加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	for (int i = 0; i < sizeof(del_playlist_1) / sizeof(char*); i++) {
		delete_story_music(del_playlist_1[i]);
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
		curr_music_data_value.which_slot = 0;
		insert_story_music(&prev_music_data_value, &curr_music_data_value);
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
		curr_music_data_value.which_slot = 0;
		insert_story_music(&prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 3 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	music_cnt = get_story_playlist_music_cnt();
	printf("-- [%d]\n", music_cnt);
	for (int j = 0; j < music_cnt; j++) {
		get_story_next_music();
	}
	printf("-------------\n");
	for (int j = 0; j < music_cnt; j++) {
		get_story_prev_music();
	}
	printf("---------------------------------------\n");
}

void build_album_favorite_playlist() {
	const char* channel_1_0 = "chan_1_0";
	const char* playlist_1_0[] = {
		"0A",
		"0B",
		"0C",
		"0D",
	};

	const char* channel_1_1 = "chan_1_1";
	const char* playlist_1_1[] = {
		"1E",
		"1F",
		"1G",
		"1H",
	};

	const char* channel_1_2 = "chan_1_2";
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
		"2K",
#if 0
		"2J",
		"1E",
		"0C",
		"1H",
#endif
	};

	const char* channel_2_0 = "chan_2_0";
	const char* playlist_2_0[] = {
		"3M",
		"3N",
		"3O",
		"3P",
	};

	const char* channel_2_1 = "chan_2_1";
	const char* playlist_2_1[] = {
		"4Q",
		"4R",
		"4S",
		"4T",
	};

	const char* channel_2_2 = "chan_2_2";
	const char* playlist_2_2[] = {
		"5U",
		"5V",
		"5W",
		"5X",
	};

	uint32_t which_slot = 0;
	uint32_t music_cnt = 0;
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
	which_slot = find_slot_no_by_chan_name(channel_1_0);
	printf("添加 %s 到 %d 哈希槽\n", channel_1_0, which_slot);
	for (int i = 0; i < sizeof(playlist_1_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_0[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = find_slot_no_by_chan_name(channel_1_1);
	printf("添加 %s 到 %d 哈希槽\n", channel_1_1, which_slot);
	for (int i = 0; i < sizeof(playlist_1_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_1[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = find_slot_no_by_chan_name(channel_1_2);
	printf("添加 %s 到 %d 哈希槽\n", channel_1_2, which_slot);
	for (int i = 0; i < sizeof(playlist_1_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1_2[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次添加歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");


	// 开始第一次删除歌曲，这里用首字母做哈希key，只是为了减少点代码
	for (int i = 0; i < sizeof(del_playlist_1) / sizeof(char*); i++) {
		delete_album_music_in_slot(del_playlist_1[i][0], del_playlist_1[i]);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 1 次删除歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	// 开始第二次增加歌曲
	get_album_playlist_header(&header_data_value);

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	which_slot = find_slot_no_by_chan_name(channel_2_0);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_0, which_slot);
	for (int i = 0; i < sizeof(playlist_2_0) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_0[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, "1E", sizeof(prev_music_data_value.path));
	which_slot = find_slot_no_by_chan_name(channel_2_1);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_1, which_slot);
	for (int i = 0; i < sizeof(playlist_2_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_1[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	strncpy(prev_music_data_value.path, "2J", sizeof(prev_music_data_value.path));
	which_slot = find_slot_no_by_chan_name(channel_2_2);
	printf("添加 %s 到 %d 哈希槽\n", channel_2_2, which_slot);
	for (int i = 0; i < sizeof(playlist_2_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2_2[i], sizeof(curr_music_data_value.path));
		curr_music_data_value.which_slot = which_slot;
		insert_album_music_in_slot(which_slot, &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

#if 1
	printf("-- 第 2 次增加歌曲 ---------------------------------------\n");
	show_album_playlist();
	printf("----------------------------------------------------------\n");

	get_album_playlist_header(&header_data_value);

	// 打印
	for (int i = 0; i < header_data_value.playlist_cnt; ++i) {
		music_cnt = get_album_music_cnt_in_slot(i);
		printf("---- album [%d] is %s, has %d music.\n", i, header_data_value.playlist[i].name, music_cnt);
		for (int j = 0; j < music_cnt; ++j) {
			get_album_next_music_in_slot(i);
		}
	}
	printf("\n-------------\n\n");
	for (int i = 0; i < header_data_value.playlist_cnt; ++i) {
		music_cnt = get_album_music_cnt_in_slot(i);
		printf("---- album [%d] is %s, has %d music.\n", i, header_data_value.playlist[i].name, music_cnt);
		for (int j = 0; j < music_cnt; j++) {
			get_album_prev_music_in_slot(i);
		}
	}
	printf("---------------------------------------\n");
#endif
}

int test_music_playlist_main() {
	//diff_story_playlist();
	diff_album_playlist();
	//build_story_favorite_playlist();
	//build_album_favorite_playlist();

	return 0;
}
