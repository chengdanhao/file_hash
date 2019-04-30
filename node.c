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

int print_node_cb(file_node_t* node) {
	music_t* music = NULL;

	music = (music_t*)(node->data.hash_value);

	if (node->used) {
		printf("{ %s }", music->path);
	} else {
		printf("{ ----- }");
	}

exit:
	return 0;
}

int get_playlist_cb(hash_property_t* file, hash_property_t* output) {
	playlist_prop_t* file_playlist = (playlist_prop_t*)(file->prop);
	playlist_prop_t* output_playlist = (playlist_prop_t*)(output->prop);

	output_playlist->reserved = file_playlist->reserved;
	output_playlist->which_album_to_handle = file_playlist->which_album_to_handle;
}

int set_playlist_cb(hash_property_t* file, hash_property_t* input) {
	playlist_prop_t* file_playlist = (playlist_prop_t*)(file->prop);
	playlist_prop_t* input_playlist = (playlist_prop_t*)(input->prop);

	file_playlist->reserved = input_playlist->reserved;
	file_playlist->which_album_to_handle = input_playlist->which_album_to_handle;
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
	traverse_nodes(PLAYLIST_PATH, print_node_cb);
}

void get_playlist_prop() {
	hash_property_t hash_prop;
	playlist_prop_t playlist_prop;

	memset(&hash_prop, 0, sizeof(hash_property_t));
	memset(&playlist_prop, 0, sizeof(playlist_prop));

	hash_prop.prop = &playlist_prop;

	get_hash_prop(PLAYLIST_PATH, &hash_prop, get_playlist_cb);

	node_info("<GET> 0x%x %d.", playlist_prop.reserved, playlist_prop.which_album_to_handle);
}

void set_playlist_prop(playlist_prop_t* playlist_prop) {
	hash_property_t hash_prop;

	node_info("<SET> 0x%x %d.", playlist_prop->reserved, playlist_prop->which_album_to_handle);

	memset(&hash_prop, 0, sizeof(hash_property_t));

	hash_prop.prop = playlist_prop;

	set_hash_prop(PLAYLIST_PATH, &hash_prop, set_playlist_cb);
}

