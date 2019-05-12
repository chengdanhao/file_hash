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
		"KKK",
		"JJJ",
		"III",
		/*"FFF",
		"BBB",
		"CCC",
		"EEE",
		"DDD",*/
		
	};

	const char* playlist_2[] = {
		"AAA",
		"BBB",
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

	playlist_header_data_value_t header_data_value;
	music_data_value_t prev_music_data_value;
	music_data_value_t curr_music_data_value;

	memset(&header_data_value, 0, sizeof(header_data_value));
	memset(&prev_music_data_value, 0, sizeof(prev_music_data_value));
	memset(&curr_music_data_value, 0, sizeof(curr_music_data_value));

	init_story_playlist_hash_engine();

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_1[i], sizeof(curr_music_data_value.path));
		printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
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

	strncpy(prev_music_data_value.path, "DDD", sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_2) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_2[i], sizeof(curr_music_data_value.path));
		printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(playlist_2[i][0], &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 2 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");

	strncpy(prev_music_data_value.path, DUMMY_MUSIC_PATH, sizeof(prev_music_data_value.path));
	for (int i = 0; i < sizeof(playlist_3) / sizeof(char*); i++) {
		curr_music_data_value.delete_or_not = MUSIC_KEEP;
		strncpy(curr_music_data_value.path, playlist_3[i], sizeof(curr_music_data_value.path));
		printf("prev(%s) -> curr(%s)\n", prev_music_data_value.path, curr_music_data_value.path);
		add_story_music(playlist_3[i][0], &prev_music_data_value, &curr_music_data_value);
		prev_music_data_value = curr_music_data_value;
	}

	printf("-- 第 3 次增加歌曲 ---------------------------------------\n");
	show_story_playlist();
	printf("----------------------------------------------------------\n");
#if 0

	get_story_playlist_header(&header_data_value);

	printf("-- %d\n", header_data_value.playlist[0].music_cnt);
	for (int j = 0; j < header_data_value.playlist[0].music_cnt; j++) {
		get_story_next_music(0);
	}
	printf("-------------\n");

	for (int j = 0; j < header_data_value.playlist[0].music_cnt; j++) {
		get_story_prev_music(0);
	}
	printf("---------------------------------------\n");
#endif
}
#if 0
uint32_t find_album_playlist_hash_slot(const char* playlist_name) {
	uint32_t playlist_no = 0;
	uint32_t playlist_cnt;
	uint32_t hash_key = 0;
	playlist_header_data_value_t header_data_value;
	uint8_t do_find_playlist = 0;

	memset(&header_data_value, 0, sizeof(header_data_value));

	get_album_playlist_header(&header_data_value);

	playlist_no = header_data_value.which_playlist_to_handle;
	playlist_cnt = header_data_value.playlist_cnt;

	for (int i = 0; i < playlist_cnt; i++) {
		if (0 == strcmp(header_data_value.playlist[i].name, playlist_name)) {
			hash_key = i;
			do_find_playlist = 1;
			break;
		}
	}

	if (0 == do_find_playlist) {
		strncpy(header_data_value.playlist[playlist_no].name, playlist_name, MAX_PLAYLIST_NAME_LEN);
		hash_key = playlist_no;
		header_data_value.which_playlist_to_handle = (playlist_no + 1) % playlist_cnt;
		set_album_playlist_header(&header_data_value);
	}

	return hash_key;
}

void build_album_favorite_playlist() {
	const char* playlist_name_0 = "channel000";
	const char* playlist_list_0_add_1[] = {
		"00A",
		"00B",
		"00C",
		"00D",
		"00E",
	};

	const char* playlist_list_0_del_1[] = {
		"00A",
		"00C",
		"00E",
	};

	const char* playlist_list_0_add_2[] = {
		"00D",
		"00F",
		"00G",
	};

	const char* playlist_name_1 = "channel001";
	const char* playlist_list_1[] = {
		"11A",
		"11B",
		"11C",
	};

	const char* playlist_name_2 = "channel002";
	const char* playlist_list_2[] = {
		"22A",
		"22B",
		"22C",
	};

	const char* playlist_name_3 = "channel003";
	const char* playlist_list_3[] = {
		"33A",
		"33B",
		"33C",
	};

	const char* playlist_name_4 = "channel004";
	const char* playlist_list_4[] = {
		"44A",
		"44B",
		"44C",
		"44D",
		"44E",
	};

	const char* playlist_name_5 = "channel005";
	const char* playlist_list_5[] = {
		"55A",
		"55B",
		"55C",
		"55D",
		"55E",
		"55F",
	};

	const char* playlist_name_6 = "channel006";
	const char* playlist_list_6[] = {
		"66A",
		"66B",
		"66C",
		"66D",
		"66E",
		"66F",
		"66G",
	};

	uint32_t hash_key = 0;

	playlist_header_data_value_t header_data_value;

	memset(&header_data_value, 0, sizeof(header_data_value));

	// 1. 初始化哈希引擎
	init_album_playlist_hash_engine();

	// 3. 初始化播放列表
	get_album_playlist_header(&header_data_value);
	header_data_value.playlist_cnt = ALBUM_SLOT_CNT;
	set_album_playlist_header(&header_data_value);

	/** START 添加三个专辑的歌曲 ***************/
	// 第一个专辑增和删
	hash_key = find_album_playlist_hash_slot(playlist_name_0);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_add_1) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_0_add_1[i]);
	}
	clean_album_playlist(hash_key);

	//_reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_del_1) / sizeof(char*); i++) {
		del_album_music(hash_key, playlist_list_0_del_1[i]);
	}
	//_clean_playlist(hash_key);

	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_add_2) / sizeof(char*); i++) {
		add_album_music(hash_key, (playlist_list_0_add_2)[i]);
	}
	clean_album_playlist(hash_key);

	// 添加第二个专辑
	hash_key = find_album_playlist_hash_slot(playlist_name_1);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_1) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_1[i]);
	}
	clean_album_playlist(hash_key);

	// 添加第三个专辑
	hash_key = find_album_playlist_hash_slot(playlist_name_2);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_2) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_2[i]);
	}
	clean_album_playlist(hash_key);
	/** END 添加三个专辑的歌曲 ***************/

	printf("-- 专辑更新前 ---------------------------------------\n");
	show_album_playlist();
	printf("-----------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_album_playlist_hash_slot(playlist_name_3);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_3) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_3[i]);
	}
	clean_album_playlist(hash_key);

	hash_key = find_album_playlist_hash_slot(playlist_name_4);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_4) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_4[i]);
	}
	clean_album_playlist(hash_key);
	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第一次更新 ---------------------------------------\n");
	show_album_playlist();
	printf("---------------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_album_playlist_hash_slot(playlist_name_5);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_5) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_5[i]);
	}
	clean_album_playlist(hash_key);

	hash_key = find_album_playlist_hash_slot(playlist_name_6);
	reset_album_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_6) / sizeof(char*); i++) {
		add_album_music(hash_key, playlist_list_6[i]);
	}
	clean_album_playlist(hash_key);
	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第二次更新 ---------------------------------------\n");
	show_album_playlist();
	printf("---------------------------------------------------------\n");

	get_album_playlist_header(&header_data_value);

	for (int i = 0; i < header_data_value.playlist_cnt; i++) {
		printf("-- [%d] = %d\n", i, header_data_value.playlist[i].music_cnt);
		for (int j = 0; j < header_data_value.playlist[i].music_cnt; j++) {
			get_album_next_music(i);
		}

		printf("-------------\n");

		for (int j = 0; j < header_data_value.playlist[i].music_cnt; j++) {
			get_album_prev_music(i);
		}
		printf("---------------------------------------\n");
	}
}
#endif
int test_music_playlist_main() {
	build_story_favorite_playlist();
	//build_album_favorite_playlist();

	return 0;
}
