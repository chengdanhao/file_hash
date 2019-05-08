#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "music_node.h"
#if 0
#define MUSIC_INFO 1
#define MUSIC_DBUG 1
#define MUSIC_WARN 1
#define MUSIC_EROR 1

#if MUSIC_INFO
#define music_info(fmt, ...) printf("\e[0;32m[MUSIC_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define music_info(fmt, ...)
#endif

#if MUSIC_DBUG
#define music_debug(fmt, ...) printf("\e[0m[MUSIC_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define music_debug(fmt, ...)
#endif

#if MUSIC_WARN
#define music_warn(fmt, ...) printf("\e[0;33m[MUSIC_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define music_warn(fmt, ...)
#endif

#if MUSIC_EROR
#define music_error(fmt, ...) printf("\e[0;31m[MUSIC_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define music_error(fmt, ...)
#endif

int __get_playlist_cb(hash_header_t* file, hash_header_t* output) {
	playlist_header_t* file_playlist = (playlist_header_t*)(file->data);
	playlist_header_t* output_playlist = (playlist_header_t*)(output->data);

	output_playlist->which_playlist_to_handle = file_playlist->which_playlist_to_handle;
	output_playlist->playlist_cnt = file_playlist->playlist_cnt;
	memcpy(output_playlist->playlist, file_playlist->playlist, sizeof(output_playlist->playlist));

	return 0;
}

int __set_playlist_cb(hash_header_t* file, hash_header_t* input) {
	playlist_header_t* file_playlist = (playlist_header_t*)(file->data);
	playlist_header_t* input_playlist = (playlist_header_t*)(input->data);

	file_playlist->which_playlist_to_handle = input_playlist->which_playlist_to_handle;
	file_playlist->playlist_cnt = input_playlist->playlist_cnt;
	memcpy(file_playlist->playlist, input_playlist->playlist, sizeof(input_playlist->playlist));

	return 0;
}

int _get_music_cb(file_node_t* node, file_node_t* output) {
	output->prev_offset = node->prev_offset;
	output->next_offset = node->next_offset;
	memcpy(output->data.value, node->data.value, sizeof(music_value_t));

	return 0;
}

int _add_music_cb(node_data_t* file, node_data_t* input) {
	file->key = input->key;
	memcpy(file->value, input->value, sizeof(music_value_t));

	return 0;
}

int _del_music_cb(node_data_t* file, node_data_t* input) {
	int ret = -1;
	music_value_t* file_music = (music_value_t*)(file->value);
	music_value_t* input_music = (music_value_t*)(input->value);

	if (0 == strncmp(file_music->path, input_music->path, MAX_MUSIC_PATH_LEN)) {
		file->key = 0;
		file_music->delete_or_not = MUSIC_DELETE;
		memset(file_music->path, 0, MAX_MUSIC_PATH_LEN);
		ret = 0;
	}

	return ret;
}

traverse_action_t _print_music_cb(file_node_t* node, node_data_t* input) {
	music_value_t* music = (music_value_t*)(node->data.value);

	if (node->used) {
		if (MUSIC_DELETE == music->delete_or_not) {
			printf("{ DEL : %s }", music->path);
		} else {
			printf("\e[7;37m%s\e[0m", music->path);
		}
	} else {
		printf("*");
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t _reset_playlist_cb(file_node_t* node, node_data_t* input) {
	music_value_t* file_music = (music_value_t*)(node->data.value);

	if (node->used) {
		file_music->delete_or_not = MUSIC_DELETE;
	}

	return TRAVERSE_ACTION_UPDATE;
}

traverse_action_t _clean_playlist_cb(file_node_t* node, node_data_t* input) {
	music_value_t* file_music = (music_value_t*)(node->data.value);

	if (node->used && MUSIC_DELETE == file_music->delete_or_not) {
		music_warn("delete %s.", file_music->path);
		node->used = 0;
		node->data.key = 0;
		memset(node->data.value, 0, sizeof(music_value_t));
	}

	return TRAVERSE_ACTION_UPDATE;
}

// 如果找到，返回1
traverse_action_t _find_music_cb(file_node_t* node, node_data_t* input) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	music_value_t* file_music = (music_value_t*)(node->data.value);
	music_value_t* input_music = (music_value_t*)(input->value);

	if (node->used && 0 == strcmp(file_music->path, input_music->path)) {
		file_music->delete_or_not = MUSIC_KEEP;
		action = TRAVERSE_ACTION_UPDATE | TRAVERSE_ACTION_BREAK;
	}

	return action;
}

void _get_playlist_header(const char* func, const int line, playlist_header_t* playlist_header) {
	hash_header_t hash_header;

	memset(&hash_header, 0, sizeof(hash_header_t));

	hash_header.data = playlist_header;

	get_hash_header(PLAYLIST_PATH, &hash_header, __get_playlist_cb);

#if 0
	printf("-------------------------- \e[7;32m<GET> %s : %d\e[0m --------------------------\n"
		"which_playlist_to_handle = %d, playlist_cnt = %d.\n",
		func, line,
		playlist_header->which_playlist_to_handle, playlist_header->playlist_cnt);

	for (int i = 0; i < playlist_header->playlist_cnt; i++) {
		printf("[%d] total_music = %d, prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
			playlist_header->playlist[i].music_cnt,
			playlist_header->playlist[i].prev_offset,
			playlist_header->playlist[i].next_offset,
			playlist_header->playlist[i].name);
	}

	printf("---------------------------------------"
		"---------------------------------------\n");
#endif
}

void _set_playlist_header(const char* func, const int line, playlist_header_t* playlist_header) {
	hash_header_t hash_header;

#if 0
	printf("-------------------------- \e[7;33m<SET> %s : %d\e[0m --------------------------\n"
		"which_playlist_to_handle = %d, playlist_cnt = %d\n",
		func, line,
		playlist_header->which_playlist_to_handle, playlist_header->playlist_cnt);

	for (int i = 0; i < playlist_header->playlist_cnt; i++) {
		printf("[%d] total_music = %d, prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
			playlist_header->playlist[i].music_cnt,
			playlist_header->playlist[i].prev_offset,
			playlist_header->playlist[i].next_offset,
			playlist_header->playlist[i].name);
	}

	printf("---------------------------------------"
		"---------------------------------------\n");
#endif

	memset(&hash_header, 0, sizeof(hash_header_t));

	hash_header.data = playlist_header;

	set_hash_header(PLAYLIST_PATH, &hash_header, __set_playlist_cb);
}

void get_music(uint32_t hash_key, direction_t next_or_prev) {
	off_t offset = 0;
	playlist_header_t playlist_header;
	uint32_t playlist_cnt = 0;
	uint32_t playlist_no = 0;
	uint32_t total_music_cnt = 0;
	get_node_method_t method = GET_NODE_BY_HASH_KEY;
	music_value_t music;
	file_node_t node = { .data = { .value = &music } };

	memset(&playlist_header, 0, sizeof(playlist_header));

	get_playlist_header(&playlist_header);
	playlist_cnt = playlist_header.playlist_cnt;
	playlist_no = hash_key % playlist_cnt;
	total_music_cnt = playlist_header.playlist[playlist_no].music_cnt;

	if (0 == total_music_cnt) {
		music_info("播放列表 '%s' 没有歌曲.", playlist_header.playlist[playlist_no].name);
		goto exit;
	}

next_node:
	offset = NEXT_MUSIC == next_or_prev ? playlist_header.playlist[playlist_no].next_offset : playlist_header.playlist[playlist_no].prev_offset;
	method = offset > 0 ? GET_NODE_BY_OFFSET : GET_NODE_BY_HASH_KEY;

	get_node(PLAYLIST_PATH, method, hash_key, offset, &node, _get_music_cb);

	playlist_header.playlist[playlist_no].next_offset = node.next_offset;
	playlist_header.playlist[playlist_no].prev_offset = node.prev_offset;

	playlist_header.playlist[playlist_no].saved_offset = offset;    // 保存当前播放列表播放进度
	playlist_header.saved_offset_for_all = offset;                  // 保存所有播放列表中最新的播放进度

	if (0 == strcmp(music.path, "")) {
		goto next_node;
	}

	music_info("音乐名称 = %s.", music.path);

	set_playlist_header(&playlist_header);

exit:
	return;
}

void get_prev_music(uint32_t hash_key) {
	get_music(hash_key, PREV_MUSIC);
}

void get_next_music(uint32_t hash_key) {
	get_music(hash_key, NEXT_MUSIC);
}

uint8_t find_music(uint32_t hash_key, const char* music_path) {
	node_data_t data;
	music_value_t music;

	memset(&data, 0, sizeof(data));
	memset(&music, 0, sizeof(music));

	strncpy(music.path, music_path, MAX_MUSIC_PATH_LEN);

	data.key = hash_key;
	data.value = &music;

	return traverse_nodes(PLAYLIST_PATH, TRAVERSE_SPECIFIC_HASH_KEY, hash_key, WITHOUT_PRINT, &data, _find_music_cb);
}

int add_music(uint32_t hash_key, const char* music_path) {
	int ret = -1;
	node_data_t data;
	music_value_t music;
	playlist_header_t playlist_header;
	uint32_t playlist_no = 0;

	memset(&playlist_header, 0, sizeof(playlist_header));

	if (find_music(hash_key, music_path) > 0) {
		music_debug("already exist '%s'", music_path);
		ret = 0;
		goto music_exist;
	}

	memset(&data, 0, sizeof(data));
	memset(&music, 0, sizeof(music));

	music.delete_or_not = MUSIC_KEEP;
	strncpy(music.path, music_path, sizeof(music.path));

	data.key = hash_key;
	data.value = &music;

	if (0 != (ret = add_node(PLAYLIST_PATH, &data, _add_music_cb))) {
		music_error("add failed : %s.", music_path);
		goto exit;
	}

	music_info("add success : %s.", music_path);

music_exist:
	get_playlist_header(&playlist_header);
	playlist_no = hash_key % playlist_header.playlist_cnt;
	++playlist_header.playlist[playlist_no].music_cnt;
	set_playlist_header(&playlist_header);

exit:
	return ret;
}

int del_music(uint32_t hash_key, const char* music_path) {
	int ret = -1;
	node_data_t data;
	music_value_t music;
	playlist_header_t playlist_header;
	uint32_t playlist_no = 0;

	memset(&data, 0, sizeof(data));
	memset(&music, 0, sizeof(music));

	music.delete_or_not = MUSIC_DELETE;
	strncpy(music.path, music_path, MAX_MUSIC_PATH_LEN);

	data.key = hash_key;
	data.value = &music;

	if (0 != (ret = del_node(PLAYLIST_PATH, &data, _del_music_cb))) {
		music_error("del failed : %s.", music_path);
		goto exit;
	}

	music_warn("del success : %s.", music_path);

	get_playlist_header(&playlist_header);
	playlist_no = hash_key % playlist_header.playlist_cnt;
	--playlist_header.playlist[playlist_no].music_cnt;
	set_playlist_header(&playlist_header);

exit:
	return ret;
}

void show_playlist() {
	traverse_nodes(PLAYLIST_PATH, TRAVERSE_ALL, 0, WITH_PRINT, NULL, _print_music_cb);
}

void reset_playlist(uint32_t hash_key) {
	playlist_header_t playlist_header;
	uint32_t playlist_no = 0;

	get_playlist_header(&playlist_header);
	playlist_no = hash_key % playlist_header.playlist_cnt;
	playlist_header.playlist[playlist_no].music_cnt = 0;
	set_playlist_header(&playlist_header);

	traverse_nodes(PLAYLIST_PATH, TRAVERSE_SPECIFIC_HASH_KEY, hash_key, WITHOUT_PRINT, NULL, _reset_playlist_cb);
}

void clean_playlist(uint32_t hash_key) {
	traverse_nodes(PLAYLIST_PATH, TRAVERSE_SPECIFIC_HASH_KEY, hash_key, WITHOUT_PRINT, NULL, _clean_playlist_cb);
}

void check_playlist() {
	check_hash_file(PLAYLIST_PATH);
}

void rebuild_playlist() {
	rebuild_hash_file(PLAYLIST_PATH);
}
#endif
