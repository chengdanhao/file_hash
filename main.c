#include <stdio.h>
#include <string.h>
#include "node.h"

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

	// 1. 初始化哈希引擎
	init_hash_engine(1, sizeof(music_t), sizeof(playlist_prop_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

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
}

uint32_t find_album_hash_slot(const char* album_name) {
	uint32_t album_no = 0;
	uint32_t hash_key = 0;
	playlist_prop_t prop;
	uint8_t do_find_album = 0;

	memset(&prop, 0, sizeof(prop));

	get_playlist_prop(&prop);

	album_no = prop.which_album_to_handle;

	for (int i = 0; i < FAVORITE_ALBUM_CNT; i++) {
		if (0 == strcmp(prop.album_name[i], album_name)) {
			hash_key = i;
			do_find_album = 1;
			break;
		}
	}

	if (0 == do_find_album) {
		strncpy(prop.album_name[album_no], album_name, FAVORITE_ALBUM_NAME_LEN);
		hash_key = prop.which_album_to_handle = (album_no + 1) % FAVORITE_ALBUM_CNT;
		set_playlist_prop(&prop);
	}

	return hash_key;
}

void build_album_favorite_playlist() {
	const char* album_name_0 = "channel000";
	const char* album_list_0[] = {
		"00A",
		"00B",
		"00C",
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
	};

	const char* album_name_6 = "channel006";
	const char* album_list_6[] = {
		"066A",
		"66B",
		"66C",
		"66D",
		"66E",
	};

	int hash_key = 0;

	// 1. 初始化哈希引擎，专辑收藏最多3个专辑，超过3个，新的会替换旧的
	init_hash_engine(FAVORITE_ALBUM_CNT, sizeof(music_t), sizeof(playlist_prop_t));

	// 2. 检查播放列表是否存在
	// check_playlist();	// 正式使用时使用改api
	rebuild_playlist();		// 调试时使用

	/** START 添加三个专辑的歌曲 ***************/

	hash_key = find_album_hash_slot(album_name_0);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_0) / sizeof(char*); i++) {
		add_music(hash_key, album_list_0[i]);
	}
	clean_playlist(hash_key);

	hash_key = find_album_hash_slot(album_name_1);
	reset_playlist(hash_key);
	for (int i = 0; i < sizeof(album_list_1) / sizeof(char*); i++) {
		add_music(hash_key, album_list_1[i]);
	}
	clean_playlist(hash_key);

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

}

int main() {
	build_story_favorite_playlist();
	build_album_favorite_playlist();
}
