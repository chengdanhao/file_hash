#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hash.h"
#include "node.h"

#define NODE_INFO 1
#define NODE_DBUG 1
#define NODE_WARN 1
#define NODE_EROR 1

#if NODE_INFO
#define node_info(fmt, ...) printf("\e[0;32m[NODE_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_info(fmt, ...)
#endif

#if NODE_DBUG
#define node_debug(fmt, ...) printf("\e[0m[NODE_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_debug(fmt, ...)
#endif

#if NODE_WARN
#define node_warn(fmt, ...) printf("\e[0;33m[NODE_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_warn(fmt, ...)
#endif

#if NODE_EROR
#define node_error(fmt, ...) printf("\e[0;31m[NODE_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define node_error(fmt, ...)
#endif

int add_node_cb(node_data_t* file_data, node_data_t* input_data) {
	file_data->hash_key = input_data->hash_key;
	memcpy(file_data->hash_value, input_data->hash_value, sizeof(music_t));
	//node_error(">>>> %s , %d.", ((book_t*)(input_data->hash_value))->path, ((book_t*)(input_data->hash_value))->code);
	return 0;
}

int del_node_cb(node_data_t* file_data, node_data_t* input_data) {
	int ret = -1;
	music_t* file_music = NULL;
	music_t* input_music = NULL;

	file_music = (music_t*)(file_data->hash_value);
	input_music = (music_t*)(input_data->hash_value);

	if (0 == strncmp(file_music->path, input_music->path, MUSIC_PATH_LEN)) {
		file_data->hash_key = 0;
		memset(file_music->path, 0, MUSIC_PATH_LEN);
		ret = 0;
	}

	return ret;
}

char* print_node_cb(node_data_t* file_data) {
	char* res = NULL;
	int malloc_size = sizeof(music_t) + 10;
	music_t* music = NULL;

	music = (music_t*)(file_data->hash_value);

	// print_node接口会释放cb中申请的内存
	if (NULL == (res = (char*)calloc(1, malloc_size))) {
		node_error("malloc failed.");
		goto exit;
	}

	snprintf(res, malloc_size, "{ %s }", music->path);

exit:
	return res;
}

int add_music(const char* music_path) {
	int ret = -1;
	node_data_t data;
	music_t music;

	memset(&data, 0, sizeof(data));
	memset(&music, 0, sizeof(music));

	strncpy(music.path, music_path, sizeof(music.path));

	data.hash_key = music_path[0];
	data.hash_value = &music;
	if (0 == (ret = add_node(PLAYLIST_PATH, &data, add_node_cb))) {
		node_info("add success : %s.", music_path);
	} else {
		node_error("add failed : %s.", music_path);
	}

exit:
	return ret;
}

int del_music(const char* music_path) {
	int ret = -1;
	node_data_t data;
	music_t music;

	memset(&data, 0, sizeof(data));
	memset(&music, 0, sizeof(music));

	strncpy(music.path, music_path, MUSIC_PATH_LEN);

	data.hash_key = music_path[0];
	data.hash_value = &music;

	if (0 == (ret = del_node(PLAYLIST_PATH, &data, del_node_cb))) {
		node_info("del success : %s.", music_path);
	} else {
		node_error("del failed : %s.", music_path);
	}

	return ret;
}

void show_playlist() {
	print_nodes(PLAYLIST_PATH, print_node_cb);
}

