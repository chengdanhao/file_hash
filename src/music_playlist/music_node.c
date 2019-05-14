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

int __add_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_prev_node_data) {
	music_data_value_t *file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t *input_prev_music_data_value = (music_data_value_t*)(input_prev_node_data->value);

	return strncmp(file_music_data_value->path, input_prev_music_data_value->path, MAX_MUSIC_PATH_LEN);
}

int __del_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)(input_node_data->value);

	return strncmp(file_music_data_value->path, input_music_data_value->path, MAX_MUSIC_PATH_LEN);
}

traverse_action_t __print_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	music_data_value_t* music_data_value = (music_data_value_t*)(file_node_data->value);

	if (MUSIC_DELETE == music_data_value->delete_or_not) {
		printf("{ DEL : %s }", music_data_value->path);
	} else {
		printf("\e[7;37m%s\e[0m", music_data_value->path);
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t __reset_playlist_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);

	file_music_data_value->delete_or_not = MUSIC_DELETE;

	return TRAVERSE_ACTION_UPDATE;
}

traverse_action_t __clean_playlist_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	void *addr = NULL;	// 防止在memcpy中，文件中保存的上一次指针值覆盖了当前正在运行的指针

	if (MUSIC_DELETE == file_music_data_value->delete_or_not) {
		music_warn("delete %s.", file_music_data_value->path);

		addr = file_node_data->value;
		memset(file_node_data, 0, sizeof(hash_node_data_t));

		file_node_data->value = addr;
		memset(file_node_data->value, 0, sizeof(music_data_value_t));
	}

	return TRAVERSE_ACTION_UPDATE;
}

// 如果找到，返回1
traverse_action_t __find_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)(input_node_data->value);

	if (0 == strcmp(file_music_data_value->path, input_music_data_value->path)) {
		file_music_data_value->delete_or_not = MUSIC_KEEP;
		action = TRAVERSE_ACTION_UPDATE | TRAVERSE_ACTION_BREAK;
	}

	return action;
}

int _get_playlist_header(const char* func, const int line, const char* path, playlist_header_data_value_t* header_data_value) {
	hash_header_data_t header_data;

	memset(&header_data, 0, sizeof(hash_header_data_t));

	header_data.value = header_data_value;

	get_header_data(path, &header_data);

#if 0
	printf("-------------------------- \e[7;32m<GET> %s : %d\e[0m --------------------------\n"
			"saved_offset_for_all = 0x%lX, which_playlist_to_handle = %d, playlist_cnt = %d.\n",
			func, line,
			header_data_value->saved_offset_for_all,
			header_data_value->which_playlist_to_handle,
			header_data_value->playlist_cnt);

	for (int i = 0; i < header_data_value->playlist_cnt; i++) {
		printf("[%d]  prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
				header_data_value->playlist[i].prev,
				header_data_value->playlist[i].next,
				header_data_value->playlist[i].name);
	}

	printf("---------------------------------------"
			"---------------------------------------\n");
#endif

	return 0;
}


int _set_playlist_header(const char* func, const int line, const char* path, playlist_header_data_value_t* header_data_value) {
	hash_header_data_t header_data;

	memset(&header_data, 0, sizeof(hash_header_data_t));

	header_data_value->which_playlist_to_handle %= header_data_value->playlist_cnt;
	header_data.value = header_data_value;

	set_header_data(path, &header_data);

#if 0
	printf("-------------------------- \e[7;31m<SET> %s : %d\e[0m --------------------------\n"
			"saved_offset_for_all = 0x%lX, which_playlist_to_handle = %d, playlist_cnt = %d.\n",
			func, line,
			header_data_value->saved_offset_for_all,
			header_data_value->which_playlist_to_handle,
			header_data_value->playlist_cnt);

	for (int i = 0; i < header_data_value->playlist_cnt; i++) {
		printf("[%d]  prev = 0x%lX, next = 0x%lX, playlist_name = %s.\n", i,
				header_data_value->playlist[i].prev,
				header_data_value->playlist[i].next,
				header_data_value->playlist[i].name);
	}

	printf("---------------------------------------"
			"---------------------------------------\n");
#endif

	return 0;
}

int _get_music(const char* playlist_path, uint32_t which_slot, direction_t next_or_prev) {
	int ret = -1;
	off_t offset = 0;
	playlist_header_data_value_t playlist_header;
	uint32_t playlist_no = 0;
	hash_node_t node;
	music_data_value_t music_data_value;

	memset(&node, 0, sizeof(hash_node_t));
	memset(&playlist_header, 0, sizeof(playlist_header));

	node.data.value = &music_data_value;

	_get_playlist_header(__func__, __LINE__, playlist_path, &playlist_header);

	playlist_no = which_slot % playlist_header.playlist_cnt;

	offset = NEXT_MUSIC == next_or_prev ? \
		playlist_header.playlist[playlist_no].next : \
		playlist_header.playlist[playlist_no].prev;

	get_node(playlist_path, which_slot, offset, &node);

	if (node.offsets.logic_prev == node.offsets.logic_next
			&& 0 == strcmp("", music_data_value.path)) {
		music_warn("no music in slot %d.", which_slot);
		goto exit;
	}

	playlist_header.playlist[playlist_no].next = node.offsets.logic_next;
	playlist_header.playlist[playlist_no].prev = node.offsets.logic_prev;

	playlist_header.playlist[playlist_no].saved_offset = offset;    // 保存当前播放列表播放进度
	playlist_header.saved_offset_for_all = offset;                  // 保存所有播放列表中最新的播放进度

	music_info("音乐名称 = %s.", music_data_value.path);

	_set_playlist_header(__func__, __LINE__, playlist_path, &playlist_header);

	ret = 0;

exit:
	return ret;
}
uint8_t _find_music(const char* playlist_path, uint32_t which_slot, const char* music_path) {
	hash_node_data_t node_data;
	music_data_value_t music_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));

	strncpy(music_data_value.path, music_path, MAX_MUSIC_PATH_LEN);

	node_data.key = which_slot;
	node_data.value = &music_data_value;

	return traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_SLOT, TRAVERSE_BY_PHYSIC,
		which_slot, WITHOUT_PRINT, &node_data, __find_music_cb);
}

int _add_music(const char* playlist_path, uint32_t which_slot,
		const music_data_value_t* prev_music_data_value,
		const music_data_value_t* curr_music_data_value) {
	int ret = -1;
	hash_node_data_t prev_node_data;
	hash_node_data_t curr_node_data;
	playlist_header_data_value_t playlist_header_data_value;

	if (_find_music(playlist_path, which_slot, curr_music_data_value->path) > 0) {
		music_debug("already exist '%s'", curr_music_data_value->path);
		ret = 0;
		goto music_exist;
	}

	memset(&prev_node_data, 0, sizeof(prev_node_data));
	memset(&curr_node_data, 0, sizeof(curr_node_data));
	memset(&playlist_header_data_value, 0, sizeof(playlist_header_data_value));

	prev_node_data.key = which_slot;
	prev_node_data.value = (void*)prev_music_data_value;

	curr_node_data.key = which_slot;
	curr_node_data.value = (void*)curr_music_data_value;

	// 第一个节点
	if (0 == strcmp(DUMMY_MUSIC_PATH, prev_music_data_value->path)) {
		curr_node_data.is_first_node = true;
	}

	if (0 != (ret = add_node(playlist_path, &prev_node_data, &curr_node_data, __add_music_cb))) {
		music_error("add failed : %s.", curr_music_data_value->path);
		goto exit;
	}

	music_info("add success : %s.", curr_music_data_value->path);

music_exist:

exit:
	return ret;
}

int _del_music(const char* playlist_path, uint32_t which_slot, const char* path) {
	int ret = -1;
	hash_node_data_t node_data;
	music_data_value_t music_data_value;

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));

	music_data_value.delete_or_not = MUSIC_DELETE;
	strncpy(music_data_value.path, path, MAX_MUSIC_PATH_LEN);

	node_data.key = which_slot;
	node_data.value = &music_data_value;

	if (0 != (ret = del_node(playlist_path, &node_data, __del_music_cb))) {
		music_error("del failed : %s.", path);
		goto exit;
	}

	music_info("del success : %s.", path);

exit:
	return ret;
}

void _show_playlist(const char* playlist_path) {
	//traverse_nodes(playlist_path, TRAVERSE_ALL, TRAVERSE_BY_PHYSIC, 0, WITH_PRINT, NULL, __print_music_cb);
	traverse_nodes(playlist_path, TRAVERSE_ALL, TRAVERSE_BY_LOGIC, 0, WITH_PRINT, NULL, __print_music_cb);
}

#if 0
void _reset_playlist(const char* playlist_path, uint32_t hash_key) {
	playlist_header_data_value_t playlist_header_data_value;

	traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_SLOT, hash_key,
			WITHOUT_PRINT, NULL, __reset_playlist_cb);
}

void _clean_playlist(const char* playlist_path, uint32_t hash_key) {
	traverse_nodes(playlist_path, TRAVERSE_SPECIFIC_HASH_SLOT, hash_key,
			WITHOUT_PRINT, NULL, __clean_playlist_cb);
}
#endif

int _init_playlist_hash_engine(const char* playlist_path, uint32_t slot_cnt) {
	playlist_header_data_value_t playlist_header;

	memset(&playlist_header, 0, sizeof(playlist_header_data_value_t));

	init_hash_engine(playlist_path, FORCE_INIT,
			slot_cnt, sizeof(music_data_value_t), sizeof(playlist_header_data_value_t));

	_get_playlist_header(__func__, __LINE__, playlist_path, &playlist_header);
	playlist_header.playlist_cnt = slot_cnt;
	_set_playlist_header(__func__, __LINE__, playlist_path, &playlist_header);

	return 0;
}
