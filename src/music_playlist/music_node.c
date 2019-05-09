#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "music_node.h"

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

int __get_playlist_cb(hash_header_data_t* file_header_data, hash_header_data_t* output_header_data) {
	playlist_header_data_value_t* file_playlist_header_data = (playlist_header_data_value_t*)(file_header_data->value);
	playlist_header_data_value_t* output_playlist_header_data = (playlist_header_data_value_t*)(output_header_data->value);

	output_playlist_header_data->which_playlist_to_handle = file_playlist_header_data->which_playlist_to_handle;
	output_playlist_header_data->playlist_cnt = file_playlist_header_data->playlist_cnt;
	memcpy(output_playlist_header_data->playlist, file_playlist_header_data->playlist, sizeof(output_playlist_header_data->playlist));

	return 0;
}

int __set_playlist_cb(hash_header_data_t* file_header_data, hash_header_data_t* input_header_data) {
	playlist_header_data_value_t* file_playlist_header_data = (playlist_header_data_value_t*)(file_header_data->value);
	playlist_header_data_value_t* input_playlist_header_data = (playlist_header_data_value_t*)(input_header_data->value);

	file_playlist_header_data->which_playlist_to_handle = input_playlist_header_data->which_playlist_to_handle;
	file_playlist_header_data->playlist_cnt = input_playlist_header_data->playlist_cnt;
	memcpy(file_playlist_header_data->playlist, input_playlist_header_data->playlist, sizeof(input_playlist_header_data->playlist));

	return 0;
}

int __get_music_cb(hash_node_t* file_node, hash_node_t* output_node) {
	output_node->prev_offset = file_node->prev_offset;
	output_node->next_offset = file_node->next_offset;
	memcpy(output_node->data.value, file_node->data.value, sizeof(music_data_value_t));

	return 0;
}

int __add_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	file_node_data->key = input_node_data->key;
	memcpy(file_node_data->value, input_node_data->value, sizeof(music_data_value_t));

	return 0;
}

int __del_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	int ret = -1;
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)(input_node_data->value);

	if (0 == strncmp(file_music_data_value->path, input_music_data_value->path, MAX_MUSIC_PATH_LEN)) {
		file_node_data->key = 0;
		file_music_data_value->delete_or_not = MUSIC_DELETE;
		memset(file_music_data_value->path, 0, MAX_MUSIC_PATH_LEN);
		ret = 0;
	}

	return ret;
}

traverse_action_t __print_music_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	music_data_value_t* music_data_value = (music_data_value_t*)(file_node->data.value);

	if (file_node->used) {
		if (MUSIC_DELETE == music_data_value->delete_or_not) {
			printf("{ DEL : %s }", music_data_value->path);
		} else {
			printf("\e[7;37m%s\e[0m", music_data_value->path);
		}
	} else {
		printf("*");
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t __reset_playlist_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node->data.value);

	if (file_node->used) {
		file_music_data_value->delete_or_not = MUSIC_DELETE;
	}

	return TRAVERSE_ACTION_UPDATE;
}

traverse_action_t __clean_playlist_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node->data.value);

	if (file_node->used && MUSIC_DELETE == file_music_data_value->delete_or_not) {
		music_warn("delete %s.", file_music_data_value->path);
		file_node->used = 0;
		file_node->data.key = 0;
		memset(file_node->data.value, 0, sizeof(music_data_value_t));
	}

	return TRAVERSE_ACTION_UPDATE;
}

// 如果找到，返回1
traverse_action_t __find_music_cb(hash_node_t* file_node, hash_node_data_t* input_node_data) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node->data.value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)(input_node_data->value);

	if (file_node->used && 0 == strcmp(file_music_data_value->path, input_music_data_value->path)) {
		file_music_data_value->delete_or_not = MUSIC_KEEP;
		action = TRAVERSE_ACTION_UPDATE | TRAVERSE_ACTION_BREAK;
	}

	return action;
}

void _get_playlist_header(const char* func, const int line, const char* playlist_path,
		playlist_header_data_value_t* playlist_header_data_value) {
	hash_header_data_t hash_header_data;

	memset(&hash_header_data, 0, sizeof(hash_header_data_t));

	hash_header_data.value = playlist_header_data_value;

	get_header(playlist_path, &hash_header_data, __get_playlist_cb);

#if 0
	printf("-------------------------- \e[7;32m<GET> %s : %d\e[0m --------------------------\n"
			"which_playlist_to_handle = %d, playlist_cnt = %d.\n",
			func, line,
			playlist_header_data_value->which_playlist_to_handle,
			playlist_header_data_value->playlist_cnt);

	for (int i = 0; i < playlist_header_data_value->playlist_cnt; i++) {
		printf("[%d] total_music = %d, prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
				playlist_header_data_value->playlist[i].music_cnt,
				playlist_header_data_value->playlist[i].prev_offset,
				playlist_header_data_value->playlist[i].next_offset,
				playlist_header_data_value->playlist[i].name);
	}

	printf("---------------------------------------"
			"---------------------------------------\n");
#endif
}

void _set_playlist_header(const char* func, const int line, const char* playlist_path,
		playlist_header_data_value_t* playlist_header_data_value) {
	hash_header_data_t hash_header_data;

#if 0
	printf("-------------------------- \e[7;33m<SET> %s : %d\e[0m --------------------------\n"
			"which_playlist_to_handle = %d, playlist_cnt = %d\n",
			func, line,
			playlist_header_data_value->which_playlist_to_handle,
			playlist_header_data_value->playlist_cnt);

	for (int i = 0; i < playlist_header_data_value->playlist_cnt; i++) {
		printf("[%d] total_music = %d, prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
				playlist_header_data_value->playlist[i].music_cnt,
				playlist_header_data_value->playlist[i].prev_offset,
				playlist_header_data_value->playlist[i].next_offset,
				playlist_header_data_value->playlist[i].name);
	}

	printf("---------------------------------------"
			"---------------------------------------\n");
#endif

	memset(&hash_header_data, 0, sizeof(hash_header_data_t));

	hash_header_data.value = playlist_header_data_value;

	set_header(playlist_path, &hash_header_data, __set_playlist_cb);
}

void _get_music(const char* playlist_path, uint32_t hash_key, direction_t next_or_prev) {
	off_t offset = 0;
	playlist_header_data_value_t playlist_header_data_value;
	uint32_t playlist_cnt = 0;
	uint32_t playlist_no = 0;
	uint32_t total_music_cnt = 0;
	get_node_method_t method = GET_NODE_BY_HASH_KEY;
	music_data_value_t music_data_value;
	hash_node_t node = { .data = { .value = &music_data_value } };

	memset(&playlist_header_data_value, 0, sizeof(playlist_header_data_value));

	_get_playlist_header(__func__, __LINE__, playlist_path, &playlist_header_data_value);
	playlist_cnt = playlist_header_data_value.playlist_cnt;
	playlist_no = hash_key % playlist_cnt;
	total_music_cnt = playlist_header_data_value.playlist[playlist_no].music_cnt;

	if (0 == total_music_cnt) {
		music_info("播放列表 '%s' 没有歌曲.", playlist_header_data_value.playlist[playlist_no].name);
		goto exit;
	}

next_node:
	offset = NEXT_MUSIC == next_or_prev ? \
		playlist_header_data_value.playlist[playlist_no].next_offset : \
		playlist_header_data_value.playlist[playlist_no].prev_offset;
	method = offset > 0 ? GET_NODE_BY_OFFSET : GET_NODE_BY_HASH_KEY;

	get_node(playlist_path, method, hash_key, offset, &node, __get_music_cb);

	playlist_header_data_value.playlist[playlist_no].next_offset = node.next_offset;
	playlist_header_data_value.playlist[playlist_no].prev_offset = node.prev_offset;

	playlist_header_data_value.playlist[playlist_no].saved_offset = offset;    // 保存当前播放列表播放进度
	playlist_header_data_value.saved_offset_for_all = offset;                  // 保存所有播放列表中最新的播放进度

	if (0 == strcmp(music_data_value.path, "")) {
		goto next_node;
	}

	music_info("音乐名称 = %s.", music_data_value.path);

	_set_playlist_header(__func__, __LINE__, playlist_path, &playlist_header_data_value);

exit:
	return;
}

uint8_t _find_music(const char* playlist_path, uint32_t hash_key, const char* music_path) {
	hash_node_data_t node_data;
	music_data_value_t music_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));

	strncpy(music_data_value.path, music_path, MAX_MUSIC_PATH_LEN);

	node_data.key = hash_key;
	node_data.value = &music_data_value;

	return traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_KEY, hash_key,
		WITHOUT_PRINT, &node_data, __find_music_cb);
}

int _add_music(const char* playlist_path, uint32_t hash_key, const char* music_path) {
	int ret = -1;
	hash_node_data_t node_data;
	music_data_value_t music_data_value;
	playlist_header_data_value_t playlist_header_data_value;
	uint32_t playlist_no = 0;

	memset(&playlist_header_data_value, 0, sizeof(playlist_header_data_value));

	if (_find_music(playlist_path, hash_key, music_path) > 0) {
		music_debug("already exist '%s'", music_path);
		ret = 0;
		goto music_exist;
	}

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));

	music_data_value.delete_or_not = MUSIC_KEEP;
	strncpy(music_data_value.path, music_path, sizeof(music_data_value.path));

	node_data.key = hash_key;
	node_data.value = &music_data_value;

	if (0 != (ret = add_node(playlist_path, &node_data, __add_music_cb))) {
		music_error("add failed : %s.", music_path);
		goto exit;
	}

	music_info("add success : %s.", music_path);

music_exist:
	_get_playlist_header(__func__, __LINE__, playlist_path, &playlist_header_data_value);
	playlist_no = hash_key % playlist_header_data_value.playlist_cnt;
	++playlist_header_data_value.playlist[playlist_no].music_cnt;
	_set_playlist_header(__func__, __LINE__, playlist_path, &playlist_header_data_value);

exit:
	return ret;
}

int _del_music(const char* playlist_path, uint32_t hash_key, const char* music_path) {
	int ret = -1;
	hash_node_data_t node_data;
	music_data_value_t music_data_value;
	playlist_header_data_value_t playlist_header_data_value;
	uint32_t playlist_no = 0;

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));

	music_data_value.delete_or_not = MUSIC_DELETE;
	strncpy(music_data_value.path, music_path, MAX_MUSIC_PATH_LEN);

	node_data.key = hash_key;
	node_data.value = &music_data_value;

	if (0 != (ret = del_node(playlist_path, &node_data, __del_music_cb))) {
		music_error("del failed : %s.", music_path);
		goto exit;
	}

	music_warn("del success : %s.", music_path);

	_get_playlist_header(__func__, __LINE__, playlist_path,&playlist_header_data_value);
	playlist_no = hash_key % playlist_header_data_value.playlist_cnt;
	--playlist_header_data_value.playlist[playlist_no].music_cnt;
	_set_playlist_header(__func__, __LINE__, playlist_path,&playlist_header_data_value);

exit:
	return ret;
}

void _show_playlist(const char* playlist_path) {
	traverse_nodes(playlist_path, TRAVERSE_ALL, 0, WITH_PRINT, NULL, __print_music_cb);
}

void _reset_playlist(const char* playlist_path, uint32_t hash_key) {
	playlist_header_data_value_t playlist_header_data_value;
	uint32_t playlist_no = 0;

	_get_playlist_header(__func__, __LINE__, playlist_path,&playlist_header_data_value);
	playlist_no = hash_key % playlist_header_data_value.playlist_cnt;
	playlist_header_data_value.playlist[playlist_no].music_cnt = 0;
	_set_playlist_header(__func__, __LINE__, playlist_path,&playlist_header_data_value);

	traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_KEY, hash_key,
		WITHOUT_PRINT, NULL, __reset_playlist_cb);
}

void _clean_playlist(const char* playlist_path, uint32_t hash_key) {
	traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_KEY, hash_key,
			WITHOUT_PRINT, NULL, __clean_playlist_cb);
}

int _init_playlist_hash_engine(const char* playlist_path, uint32_t slot_cnt) {
	return init_hash_engine(playlist_path, FORCE_INIT,
			slot_cnt, sizeof(music_data_value_t), sizeof(playlist_header_data_value_t));
}
