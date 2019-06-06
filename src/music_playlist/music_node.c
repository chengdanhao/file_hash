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

typedef struct {
	uint32_t which_slot;
	char *download_list_path;
	char *delete_list_path;
} download_and_delete_info_t;

#define DEBUG_LIST 0
traverse_action_t __show_playlist_cb(hash_node_data_t* file_node_data, void* input_arg) {
	music_data_value_t* music_data_value = (music_data_value_t*)(file_node_data->value);

#if DEBUG_LIST
	if (MUSIC_TO_BE_DELETE == music_data_value->delete_or_not) {
		printf("{ 删除 : %s }", music_data_value->path);
	} else if (MUSIC_TO_BE_DOWNLOAD == music_data_value->delete_or_not) {
		printf("{ 下载 : %s }", music_data_value->path);
	} else if (MUSIC_KEEP == music_data_value->delete_or_not) {
		printf("{ 保留 : %s }", music_data_value->path);
	}
#else
	printf("\e[7;37m%s\e[0m", music_data_value->path);
#endif
	return TRAVERSE_ACTION_DO_NOTHING;
}
#undef DEBUG_LIST

traverse_action_t __clean_playlist_cb(hash_node_data_t* file_node_data, void* input_arg) {
	music_data_value_t* music_data_value = (music_data_value_t*)(file_node_data->value);
	char* delete_list_path = (char*)input_arg;

	_delete_music(delete_list_path, music_data_value->which_slot, music_data_value->path);

	// TODO:删除文件

	return TRAVERSE_ACTION_DO_NOTHING;
}

traverse_action_t __pre_diff_playlist_cb(hash_node_data_t* file_node_data, void* input_arg) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);

	file_music_data_value->delete_or_not = MUSIC_TO_BE_DELETE;

	return TRAVERSE_ACTION_UPDATE;
}

traverse_action_t __build_download_and_delete_list_cb(hash_node_data_t* file_node_data, void* input_arg) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	download_and_delete_info_t* info = (download_and_delete_info_t*)input_arg;
	uint32_t which_slot = info->which_slot;
	char* download_list_path = info->download_list_path;
	char* delete_list_path = info->delete_list_path;

	if (MUSIC_TO_BE_DELETE == file_music_data_value->delete_or_not) {
		music_debug("删除 %s.", file_music_data_value->path);
		music_data_value_t prev_music_data_value;

		memset(&prev_music_data_value, 0, sizeof(music_data_value_t));

		_get_first_node(delete_list_path, which_slot, &prev_music_data_value);
		_insert_music(delete_list_path, which_slot, &prev_music_data_value, file_music_data_value);
	}

	else if (MUSIC_TO_BE_DOWNLOAD == file_music_data_value->delete_or_not) {
		music_debug("下载 %s.", file_music_data_value->path);
		music_data_value_t prev_music_data_value;

		memset(&prev_music_data_value, 0, sizeof(music_data_value_t));

		_get_first_node(download_list_path, which_slot, &prev_music_data_value);
		_insert_music(download_list_path, which_slot, &prev_music_data_value, file_music_data_value);
	}

	else if (MUSIC_KEEP == file_music_data_value->delete_or_not) {
		music_debug("保留 %s.", file_music_data_value->path);
	}

	return TRAVERSE_ACTION_DO_NOTHING;
}

// 找到节点后，会将节点标记为MUSIC_KEEP
traverse_action_t __find_music_cb(hash_node_data_t* file_node_data, void* input_arg) {
	int action = TRAVERSE_ACTION_DO_NOTHING;
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)input_arg;

	if (0 == strcmp(file_music_data_value->path, input_music_data_value->path)) {
		file_music_data_value->delete_or_not = MUSIC_KEEP;
		action = TRAVERSE_ACTION_UPDATE | TRAVERSE_ACTION_BREAK;
	}

	return action;
}

bool __add_music_cb(		hash_node_data_t* file_node_data, hash_node_data_t* input_prev_node_data) {
	music_data_value_t *file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t *input_prev_music_data_value = (music_data_value_t*)(input_prev_node_data->value);

	return 0 == strncmp(file_music_data_value->path, input_prev_music_data_value->path, MAX_MUSIC_PATH_LEN);
}

bool __del_music_cb(hash_node_data_t* file_node_data, hash_node_data_t* input_node_data) {
	music_data_value_t* file_music_data_value = (music_data_value_t*)(file_node_data->value);
	music_data_value_t* input_music_data_value = (music_data_value_t*)(input_node_data->value);

	return 0 == strncmp(file_music_data_value->path, input_music_data_value->path, MAX_MUSIC_PATH_LEN);
}

void _show_playlist(const char* list_path) {
	traverse_nodes(list_path, TRAVERSE_BY_PHYSIC,
				MAX_HASH_SLOT_CNT, WITH_PRINT, NULL, __show_playlist_cb);
	traverse_nodes(list_path, TRAVERSE_BY_LOGIC,
			MAX_HASH_SLOT_CNT, WITH_PRINT, NULL, __show_playlist_cb);
}

// 将music的delete_or_not标记设置为MUSIC_DELETE
void _pre_diff_playlist(const char* list_path,
		const char* download_list_path,
		const char* delete_list_path) {
	traverse_nodes(list_path, TRAVERSE_BY_LOGIC,
			MAX_HASH_SLOT_CNT, WITHOUT_PRINT, NULL, __pre_diff_playlist_cb);

	_init_music_hash_engine(download_list_path, ALBUM_SLOT_CNT);
	_init_music_hash_engine(delete_list_path, ALBUM_SLOT_CNT);
}

void _post_diff_playlist(const char* list_path,
		const char* download_list_path,
		const char* delete_list_path) {
	download_and_delete_info_t input_arg;
	playlist_header_data_value_t playlist_header;

	memset(&playlist_header, 0, sizeof(playlist_header));
	memset(&input_arg, 0, sizeof(download_and_delete_info_t));

	_get_playlist_header(__func__, __LINE__, list_path, &playlist_header);

	// 生成链表
	for (int i = 0; i < playlist_header.playlist_cnt; ++i) {
		input_arg.which_slot = i;
		input_arg.download_list_path = (char*)download_list_path;
		input_arg.delete_list_path = (char*)delete_list_path;
		traverse_nodes(list_path, TRAVERSE_BY_LOGIC,
				i, WITHOUT_PRINT, &input_arg, __build_download_and_delete_list_cb);
	}

	// 根据删除列表的功能删除节点和文件
	traverse_nodes(delete_list_path, TRAVERSE_BY_LOGIC,
			MAX_HASH_SLOT_CNT, WITHOUT_PRINT, (void*)list_path, __clean_playlist_cb);
}

int _get_playlist_music_cnt(const char* list_path, uint32_t which_slot) {
	return get_slot_node_cnt(list_path, which_slot);
}

int _get_first_node(const char* list_path, uint32_t which_slot, music_data_value_t* music_data_value) {
	hash_node_t prev_node;

	memset(&prev_node, 0, sizeof(hash_node_t));

	prev_node.data.value = music_data_value;

	return get_node(list_path, which_slot, 0, &prev_node);
}

uint8_t _find_music(const char* list_path, uint32_t which_slot, const char* music_path) {
	music_data_value_t music_data_value;

	memset(&music_data_value, 0, sizeof(music_data_value));

	strncpy(music_data_value.path, music_path, MAX_MUSIC_PATH_LEN);

	return traverse_nodes(list_path, TRAVERSE_BY_PHYSIC,
			which_slot, WITHOUT_PRINT, &music_data_value, __find_music_cb);
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

int _get_music(const char* list_path, uint32_t which_slot, direction_t next_or_prev) {
	int ret = -1;
	off_t offset = 0;
	playlist_header_data_value_t playlist_header;
	uint32_t playlist_no = 0;
	hash_node_t node;
	music_data_value_t music_data_value;

	memset(&node, 0, sizeof(hash_node_t));
	memset(&playlist_header, 0, sizeof(playlist_header));

	node.data.value = &music_data_value;

	_get_playlist_header(__func__, __LINE__, list_path, &playlist_header);

	playlist_no = which_slot % playlist_header.playlist_cnt;

	offset = NEXT_MUSIC == next_or_prev ? \
		 playlist_header.playlist[playlist_no].next : \
		 playlist_header.playlist[playlist_no].prev;

	get_node(list_path, which_slot, offset, &node);

	if (is_slot_empty(list_path, which_slot)) {
		music_warn("no music in slot %d.", which_slot);
		goto exit;
	}

	playlist_header.playlist[playlist_no].next = node.offsets.logic_next;
	playlist_header.playlist[playlist_no].prev = node.offsets.logic_prev;

	playlist_header.playlist[playlist_no].saved_offset = offset;    // 保存当前播放列表播放进度
	playlist_header.saved_offset_for_all = offset;                  // 保存所有播放列表中最新的播放进度

	music_info("音乐名称 = %s.", music_data_value.path);

	_set_playlist_header(__func__, __LINE__, list_path, &playlist_header);

	ret = 0;

exit:
	return ret;
}

/* 该函数在 普通添加 和 diff链表可以复用
 * 普通添加将curr_music的delete_or_not标记设置为MUSIC_KEEP
 * diff链表将curr_music的delete_or_not标记设置为MUSIC_TO_BE_DOWNLOAD
 */
int _insert_music(const char* list_path, uint32_t which_slot,
		const music_data_value_t* prev_music_data_value,
		const music_data_value_t* curr_music_data_value) {
	int ret = -1;
	hash_node_data_t prev_node_data;
	hash_node_data_t curr_node_data;
	playlist_header_data_value_t playlist_header_data_value;

	// 如果存在，会将对应节点标记为MUSIC_KEEP
	if (_find_music(list_path, which_slot, curr_music_data_value->path) > 0) {
		music_debug("already exist '%s'", curr_music_data_value->path);
		ret = 0;
		goto exit;
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
		music_debug("[ + ] first node '%s' to '%s'.", curr_music_data_value->path, list_path);
		curr_node_data.is_first_node = true;
	}

	if (0 != (ret = insert_node(list_path, &prev_node_data, &curr_node_data, __add_music_cb))) {
		music_error("[ + ] '%s' to '%s' failed!", curr_music_data_value->path, list_path);
		goto exit;
	}

	music_info("[ + ] '%s' to '%s' success.", curr_music_data_value->path, list_path);

exit:
	return ret;
}

int _delete_music(const char* list_path, uint32_t which_slot, const char* path) {
	int ret = -1;
	hash_node_data_t node_data;
	music_data_value_t music_data_value;
	playlist_header_data_value_t playlist_header;

	memset(&node_data, 0, sizeof(node_data));
	memset(&music_data_value, 0, sizeof(music_data_value));
	memset(&playlist_header, 0, sizeof(playlist_header));

	_get_playlist_header(__func__, __LINE__, list_path, &playlist_header);

	music_data_value.delete_or_not = MUSIC_TO_BE_DELETE;
	strncpy(music_data_value.path, path, MAX_MUSIC_PATH_LEN);

	node_data.key = which_slot % playlist_header.playlist_cnt;
	node_data.value = &music_data_value;

	if (0 != (ret = del_node(list_path, &node_data, __del_music_cb))) {
		music_error("[ - ] '%s' from '%s' in slot '%d'.", path, list_path, node_data.key);
		goto exit;
	}

	music_info("[ - ] '%s' from '%s' success.", path, list_path);

exit:
	return ret;
}

int _init_music_hash_engine(const char* list_path, uint32_t slot_cnt) {
	playlist_header_data_value_t playlist_header;

	memset(&playlist_header, 0, sizeof(playlist_header_data_value_t));

	init_hash_engine(list_path, FORCE_INIT,
			slot_cnt, sizeof(music_data_value_t), sizeof(playlist_header_data_value_t));

	_get_playlist_header(__func__, __LINE__, list_path, &playlist_header);
	playlist_header.playlist_cnt = slot_cnt;
	_set_playlist_header(__func__, __LINE__, list_path, &playlist_header);

	return 0;
}
