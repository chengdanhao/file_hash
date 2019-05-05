#include <stdio.h>
#include <string.h>
#include "node.h"

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

	playlist_prop_t playlist_prop;

	memset(&playlist_prop, 0, sizeof(playlist_prop));

	// 1. 初始化哈希引擎
	init_hash_engine(STORY_SLOT_CNT, sizeof(music_t), sizeof(playlist_prop_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

	// 3. 初始化播放列表
	get_playlist_prop(&playlist_prop);
	playlist_prop.total_record_cnt = STORY_SLOT_CNT;
	set_playlist_prop(&playlist_prop);

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

	playlist_prop_t prop;

	memset(&prop, 0, sizeof(prop));

	get_playlist_prop(&prop);

	printf("-- %d\n", prop.record[0].total_music_cnt);
	for (int j = 0; j < prop.record[0].total_music_cnt; j++) {
		get_prev_music(0);
	}
	printf("-------------\n");

	for (int j = 0; j < prop.record[0].total_music_cnt; j++) {
		get_next_music(0);
	}
	printf("---------------------------------------\n");
}

uint32_t find_album_hash_slot(const char* album_name) {
	uint32_t album_no = 0;
	uint32_t record_cnt;
	uint32_t hash_key = 0;
	playlist_prop_t prop;
	uint8_t do_find_album = 0;

	memset(&prop, 0, sizeof(prop));

	get_playlist_prop(&prop);

	album_no = prop.which_album_to_handle;
	record_cnt = prop.total_record_cnt;

	for (int i = 0; i < record_cnt; i++) {
		if (0 == strcmp(prop.record[i].album_name, album_name)) {
			hash_key = i;
			do_find_album = 1;
			break;
		}
	}

	if (0 == do_find_album) {
		strncpy(prop.record[album_no].album_name, album_name, MAX_ALBUM_NAME_LEN);
		hash_key = album_no;
		prop.which_album_to_handle = (album_no + 1) % record_cnt;
		set_playlist_prop(&prop);
	}

	return hash_key;
}

#define ALBUM_SLOT_CNT MAX_ALBUM_CNT
void build_album_favorite_playlist() {
	const char* album_name_0 = "channel000";
	const char* album_list_0_add_1[] = {
		"00A",
		"00B",
		"00C",
		"00D",
		"00E",
	};

	const char* album_list_0_del_1[] = {
		"00A",
		"00C",
		"00E",
	};

	const char* album_list_0_add_2[] = {
		"00D",
		"00F",
		"00G",
	};

	const char* album_name_1 = "channel001";
	const char* album_list_1[] = {
		"11A",
		"11B",
		"11C",
	};

	const char* album_name_2 = "channel002";
	const char* album_list_2[] = {
		"22A",
		"22B",
		"22C",
	};

	const char* album_name_3 = "channel003";
	const char* album_list_3[] = {
		"33A",
		"33B",
		"33C",
	};

	const char* album_name_4 = "channel004";
	const char* album_list_4[] = {
		"44A",
		"44B",
		"44C",
		"44D",
		"44E",
	};

	const char* album_name_5 = "channel005";
	const char* album_list_5[] = {
		"55A",
		"55B",
		"55C",
		"55D",
		"55E",
		"55F",
	};

	const char* album_name_6 = "channel006";
	const char* album_list_6[] = {
		"66A",
		"66B",
		"66C",
		"66D",
		"66E",
		"66F",
		"66G",
	};

	uint32_t hash_key = 0;

	playlist_prop_t playlist_prop;

	memset(&playlist_prop, 0, sizeof(playlist_prop));

	// 1. 初始化哈希引擎
	init_hash_engine(ALBUM_SLOT_CNT, sizeof(music_t), sizeof(playlist_prop_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

	// 3. 初始化播放列表
	get_playlist_prop(&playlist_prop);
	playlist_prop.total_record_cnt = ALBUM_SLOT_CNT;
	set_playlist_prop(&playlist_prop);

	/** START 添加三个专辑的歌曲 ***************/

	// 第一个专辑增和删
	hash_key = find_album_hash_slot(album_name_0);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_0_add_1) / sizeof(char*); i++) {
		add_music(hash_key, album_list_0_add_1[i]);
	}
	clean_playlist(hash_key);

	//reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_0_del_1) / sizeof(char*); i++) {
		del_music(hash_key, album_list_0_del_1[i]);
	}
	//clean_playlist(hash_key);

	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_0_add_2) / sizeof(char*); i++) {
		add_music(hash_key, (album_list_0_add_2)[i]);
	}
	clean_playlist(hash_key);

	// 添加第二个专辑
	hash_key = find_album_hash_slot(album_name_1);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_1) / sizeof(char*); i++) {
		add_music(hash_key, album_list_1[i]);
	}
	clean_playlist(hash_key);

	// 添加第三个专辑
	hash_key = find_album_hash_slot(album_name_2);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_2) / sizeof(char*); i++) {
		add_music(hash_key, album_list_2[i]);
	}
	clean_playlist(hash_key);
	/** END 添加三个专辑的歌曲 ***************/

	printf("-- 专辑更新前 ---------------------------------------\n");
	show_playlist();
	printf("-----------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_album_hash_slot(album_name_3);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_3) / sizeof(char*); i++) {
		add_music(hash_key, album_list_3[i]);
	}
	clean_playlist(hash_key);

	hash_key = find_album_hash_slot(album_name_4);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_4) / sizeof(char*); i++) {
		add_music(hash_key, album_list_4[i]);
	}
	clean_playlist(hash_key);
	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第一次更新 ---------------------------------------\n");
	show_playlist();
	printf("---------------------------------------------------------\n");

	/** START 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉 ***************/
	hash_key = find_album_hash_slot(album_name_5);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_5) / sizeof(char*); i++) {
		add_music(hash_key, album_list_5[i]);
	}
	clean_playlist(hash_key);

	hash_key = find_album_hash_slot(album_name_6);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_6) / sizeof(char*); i++) {
		add_music(hash_key, album_list_6[i]);
	}
	clean_playlist(hash_key);

	/** END 再添加两个专辑歌曲，此时最早的两个专辑会被覆盖掉*****************/

	printf("-- 专辑第二次更新 ---------------------------------------\n");
	show_playlist();
	printf("---------------------------------------------------------\n");

	playlist_prop_t prop;

	memset(&prop, 0, sizeof(prop));

	get_playlist_prop(&prop);

	for (int i = 0; i < prop.total_record_cnt; i++) {
		printf("-- [%d] = %d\n", i, prop.record[i].total_music_cnt);
		for (int j = 0; j < prop.record[i].total_music_cnt; j++) {
			get_prev_music(i);
		}
		printf("-------------\n");

		for (int j = 0; j < prop.record[i].total_music_cnt; j++) {
			get_next_music(i);
		}
		printf("---------------------------------------\n");
	}
}

int main() {
	build_story_favorite_playlist();
	build_album_favorite_playlist();
}
