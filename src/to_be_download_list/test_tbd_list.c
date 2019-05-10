#include <stdio.h>
#include <string.h>
#include "tbd_node.h"
#if 0
void build_tbd_list() {
	tbd_node_data_value_t tbd_list[] = {
		{ "A_URL", "A_PATH" },
		{ "B_URL", "B_PATH" },
		{ "C_URL", "C_PATH" },
		{ "D_URL", "D_PATH" },
		{ "E_URL", "E_PATH" },
	};

	uint32_t hash_key = 0;
	char url[MAX_TBD_URL_LEN] = {0};
	char path[MAX_TBD_PATH_LEN] = {0};

	// 1. 初始化哈希引擎，不需要哈希头部，所以写0
	init_tbd_hash_engine();

	// 3. 初始化播放列表
	for (int i = 0; i < sizeof(tbd_list) / sizeof(tbd_list[0]); i++) {
		add_tbd(hash_key, tbd_list[i].url, tbd_list[i].path);
	}

	show_tbd_list();

	while (0 == get_tbd(hash_key, url, path)) {
		printf(">>> url = %s, path = %s.\n", url, path);
	}
}

int test_tbd_list_main() {
	build_tbd_list();
	return 0;
}
#endif
