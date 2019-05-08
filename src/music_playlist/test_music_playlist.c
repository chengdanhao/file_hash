#include <stdio.h>
#include <string.h>
#include "music_node.h"
#if 0
#define STORY_SLOT_CNT 1
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

	const char* playlist_2[] = {
		"AAA",
		"BBB",
		"EEE",
		"FFF",
		"JJJ",
		"UUU new",
		"VVV new",
		"WWW new",
		"XXX new",
		"YYY new",
		"ZZZ new",
	};

	playlist_header_t header;

	memset(&header, 0, sizeof(header));

	// 1. 初始化哈希引擎
	init_hash_engine(STORY_SLOT_CNT, sizeof(music_value_t), sizeof(playlist_header_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

	// 3. 初始化播放列表
	get_playlist_header(&header);
	header.playlist_cnt = STORY_SLOT_CNT;
	set_playlist_header(&header);

	for (int i = 0; i < sizeof(playlist_1) / sizeof(char*); i++) {
		add_music(playlist_1[i][0], playlist_1[i]);
	}

	printf("-- 歌曲更新前 ---------------------------------------\n");
	show_playlist();
	printf("-----------------------------------------------------\n");

	/** START 标准的添加音乐步骤 ***************/

	// 3.1. 将所有歌曲默认标记为待删除
	reset_playlist(0);

	// 3.2. 添加云端json串下发的歌曲，并将json串中的标记为已下载
	for (int i = 0; i < sizeof(playlist_2) / sizeof(char*); i++) {
		add_music(playlist_2[i][0], playlist_2[i]);
	}

	// 3.3. 清理列表中不存在的歌曲
	clean_playlist(0);

	/** END 标准添加音乐步骤*****************/

	printf("-- 歌曲更新后 ---------------------------------------\n");
	show_playlist();
	printf("-----------------------------------------------------\n");

	get_playlist_header(&header);

	printf("-- %d\n", header.playlist[0].music_cnt);
	for (int j = 0; j < header.playlist[0].music_cnt; j++) {
		get_prev_music(0);
	}
	printf("-------------\n");

	for (int j = 0; j < header.playlist[0].music_cnt; j++) {
		get_next_music(0);
	}
	printf("---------------------------------------\n");
}

uint32_t find_playlist_hash_slot(const char* playlist_name) {
	uint32_t playlist_no = 0;
	uint32_t playlist_cnt;
	uint32_t hash_key = 0;
	playlist_header_t header;
	uint8_t do_find_playlist = 0;

	memset(&header, 0, sizeof(header));

	get_playlist_header(&header);

	playlist_no = header.which_playlist_to_handle;
	playlist_cnt = header.playlist_cnt;

	for (int i = 0; i < playlist_cnt; i++) {
		if (0 == strcmp(header.playlist[i].name, playlist_name)) {
			hash_key = i;
			do_find_playlist = 1;
			break;
		}
	}

	if (0 == do_find_playlist) {
		strncpy(header.playlist[playlist_no].name, playlist_name, MAX_PLAYLIST_NAME_LEN);
		hash_key = playlist_no;
		header.which_playlist_to_handle = (playlist_no + 1) % playlist_cnt;
		set_playlist_header(&header);
	}

	return hash_key;
}

#define ALBUM_SLOT_CNT MAX_PLAYLIST_CNT
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

	playlist_header_t header;

	memset(&header, 0, sizeof(header));

	// 1. 初始化哈希引擎
	init_hash_engine(ALBUM_SLOT_CNT, sizeof(music_value_t), sizeof(playlist_header_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

	// 3. 初始化播放列表
	get_playlist_header(&header);
	header.playlist_cnt = ALBUM_SLOT_CNT;
	set_playlist_header(&header);

	/** START 添加三个专辑的歌曲 ***************/
	// 第一个专辑增和删
	hash_key = find_playlist_hash_slot(playlist_name_0);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_add_1) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_0_add_1[i]);
	}
	clean_playlist(hash_key);

	//reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_del_1) / sizeof(char*); i++) {
		del_music(hash_key, playlist_list_0_del_1[i]);
	}
	//clean_playlist(hash_key);

	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_0_add_2) / sizeof(char*); i++) {
		add_music(hash_key, (playlist_list_0_add_2)[i]);
	}
	clean_playlist(hash_key);

	// 添加第二个专辑
	hash_key = find_playlist_hash_slot(playlist_name_1);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_1) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_1[i]);
	}
	clean_playlist(hash_key);

	// 添加第三个专辑
	hash_key = find_playlist_hash_slot(playlist_name_2);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_2) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_2[i]);
	}
	clean_playlist(hash_key);
	/** END 添加三个专辑的歌曲 ***************/

	printf("-- 专辑更新前 ---------------------------------------\n");
	show_playlist();
	printf("-----------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_playlist_hash_slot(playlist_name_3);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_3) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_3[i]);
	}
	clean_playlist(hash_key);

	hash_key = find_playlist_hash_slot(playlist_name_4);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_4) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_4[i]);
	}
	clean_playlist(hash_key);
	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第一次更新 ---------------------------------------\n");
	show_playlist();
	printf("---------------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_playlist_hash_slot(playlist_name_5);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_5) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_5[i]);
	}
	clean_playlist(hash_key);

	hash_key = find_playlist_hash_slot(playlist_name_6);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(playlist_list_6) / sizeof(char*); i++) {
		add_music(hash_key, playlist_list_6[i]);
	}
	clean_playlist(hash_key);
	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第二次更新 ---------------------------------------\n");
	show_playlist();
	printf("---------------------------------------------------------\n");

	get_playlist_header(&header);

	for (int i = 0; i < header.playlist_cnt; i++) {
		printf("-- [%d] = %d\n", i, header.playlist[i].music_cnt);
		for (int j = 0; j < header.playlist[i].music_cnt; j++) {
			get_next_music(i);
		}

		printf("-------------\n");

		for (int j = 0; j < header.playlist[i].music_cnt; j++) {
			get_prev_music(i);
		}
		printf("---------------------------------------\n");
	}
}

int test_music_playlist_main() {
	build_story_favorite_playlist();
	build_album_favorite_playlist();

	return 0;
}
#endif
