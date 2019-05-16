#ifndef __MUSIC_NODE_H__
#define __MUSIC_NODE_H__

#include <stdint.h>
#include "hash.h"

#define MAX_MUSIC_PATH_LEN 200
#define MAX_PLAYLIST_NAME_LEN 20
#define MAX_HASH_SLOT_CNT 3

#define STORY_PLAYLIST_PATH "story_playlist"
#define STORY_DELETE_LIST_PATH "story_delete_list"
#define STORY_DOWNLOAD_LIST_PATH "story_download_list"

#define ALBUM_PLAYLIST_PATH "album_playlist"
#define ALBUM_DELETE_LIST_PATH "album_delete_list"
#define ALBUM_DOWNLOAD_LIST_PATH "album_download_list"

#define STORY_SLOT_CNT 1
#define ALBUM_SLOT_CNT MAX_HASH_SLOT_CNT

#define DUMMY_MUSIC_PATH "dummy_path"

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

typedef enum {
	MUSIC_TO_BE_DELETE,
	MUSIC_KEEP,
	MUSIC_TO_BE_DOWNLOAD,
	MUSIC_DO_NOTHING,
} action_t;

typedef struct {
	off_t prev;			// 记录当前节点的上一首，方便上下首切歌
	off_t next;
	off_t saved_offset;	// 记录上一次播放记录
	char name[MAX_PLAYLIST_NAME_LEN];
} playlist_t;

typedef struct {
	uint32_t playlist_cnt;
	off_t saved_offset_for_all;
	uint32_t which_playlist_to_handle;
	playlist_t playlist[MAX_HASH_SLOT_CNT];
} playlist_header_data_value_t;

typedef struct {
	action_t delete_or_not;		// 判断歌曲是否删除
	uint32_t which_slot;
	char path[MAX_MUSIC_PATH_LEN];
} music_data_value_t;

typedef enum {
	NEXT_MUSIC,
	PREV_MUSIC,
} direction_t;

void _show_playlist(const char* list_path);
void _clean_playlist(const char* list_path, const char* delete_list_path);
void _pre_diff_playlist(const char* list_path);
void _post_diff_playlist(const char* list_path, const char* download_list_path, const char* delete_list_path);
int _get_first_node(const char* list_path, uint32_t which_slot, music_data_value_t* music_data_value);
int _get_playlist_music_cnt(const char* list_path, uint32_t which_slot);
int _get_playlist_header(const char* func, const int line, const char* path, playlist_header_data_value_t* header_data_value);
int _set_playlist_header(const char* func, const int line, const char* path, playlist_header_data_value_t* header_data_value);
int _get_music(const char* list_path, uint32_t which_slot, direction_t prev_or_next);
int _add_music(const char* list_path, uint32_t which_slot, const music_data_value_t* prev_music_data_value, const music_data_value_t* curr_music_data_value);
int _del_music(const char* list_path, uint32_t which_slot, const char* path);
int _init_music_hash_engine(const char* path, uint32_t slot_cnt);

/********************** 故事收藏 调用这些函数 **********************/
#define show_story_playlist() _show_playlist(STORY_PLAYLIST_PATH)
#define show_story_download_list() _show_playlist(STORY_DOWNLOAD_LIST_PATH)
#define show_story_delete_list() _show_playlist(STORY_DELETE_LIST_PATH)

#define pre_diff_story_playlist() _pre_diff_playlist(STORY_PLAYLIST_PATH)
#define post_diff_story_playlist() _post_diff_playlist(STORY_PLAYLIST_PATH, STORY_DOWNLOAD_LIST_PATH, STORY_DELETE_LIST_PATH)

#define get_story_first_node(music_data_value) _get_first_node(STORY_PLAYLIST_PATH, 0, music_data_value)

#define get_story_playlist_music_cnt() _get_playlist_music_cnt(STORY_PLAYLIST_PATH, 0)

#define get_story_playlist_header(header_data_value) _get_playlist_header(__func__, __LINE__, STORY_PLAYLIST_PATH, header_data_value)
#define set_story_playlist_header(header_data_value) _set_playlist_header(__func__, __LINE__, STORY_PLAYLIST_PATH, header_data_value)

#define get_story_prev_music() _get_music(STORY_PLAYLIST_PATH, 0, PREV_MUSIC)
#define get_story_next_music() _get_music(STORY_PLAYLIST_PATH, 0, NEXT_MUSIC)

#define add_story_music(prev_music_data_value, curr_music_data_value) _add_music(STORY_PLAYLIST_PATH, 0, prev_music_data_value, curr_music_data_value)
#define del_story_music(music_path) _del_music(STORY_PLAYLIST_PATH, 0, music_path)

#define add_story_music_to_delete_list(prev_music_data_value, curr_music_data_value) _add_music(STORY_DELETE_LIST_PATH, 0, prev_music_data_value, curr_music_data_value)
#define add_story_music_to_download_list(prev_music_data_value, curr_music_data_value) _add_music(STORY_DOWNLOAD_LIST_PATH, 0, prev_music_data_value, curr_music_data_value)

#define init_story_playlist_hash_engine() _init_music_hash_engine(STORY_PLAYLIST_PATH, STORY_SLOT_CNT)
#define init_story_delete_list_hash_engine() _init_music_hash_engine(STORY_DELETE_LIST_PATH, STORY_SLOT_CNT)
#define init_story_download_list_hash_engine() _init_music_hash_engine(STORY_DOWNLOAD_LIST_PATH, STORY_SLOT_CNT)
/*******************************************************************/

/********************** 专辑收藏 调用这些函数 **********************/
#define show_album_playlist() _show_playlist(ALBUM_PLAYLIST_PATH)
#define show_album_download_list() _show_playlist(ALBUM_DOWNLOAD_LIST_PATH)
#define show_album_delete_list() _show_playlist(ALBUM_DELETE_LIST_PATH)

#define pre_diff_album_playlist() _pre_diff_playlist(ALBUM_PLAYLIST_PATH)
#define post_diff_album_playlist() _post_diff_playlist(ALBUM_PLAYLIST_PATH, ALBUM_DOWNLOAD_LIST_PATH, ALBUM_DELETE_LIST_PATH)

#define get_album_first_node_in_slot(which_slot, music_data_value) _get_first_node(ALBUM_PLAYLIST_PATH, which_slot, music_data_value)

#define get_album_music_cnt_in_slot(which_slot) _get_playlist_music_cnt(ALBUM_PLAYLIST_PATH, which_slot)

#define get_album_playlist_header(header_data_value) _get_playlist_header(__func__, __LINE__, ALBUM_PLAYLIST_PATH, header_data_value)
#define set_album_playlist_header(header_data_value) _set_playlist_header(__func__, __LINE__, ALBUM_PLAYLIST_PATH, header_data_value)

#define get_album_prev_music_in_slot(which_slot) _get_music(ALBUM_PLAYLIST_PATH, which_slot, PREV_MUSIC)
#define get_album_next_music_in_slot(which_slot) _get_music(ALBUM_PLAYLIST_PATH, which_slot, NEXT_MUSIC)

#define add_album_music_in_slot(which_slot, prev_music_data_value, curr_music_data_value) _add_music(ALBUM_PLAYLIST_PATH, which_slot, prev_music_data_value, curr_music_data_value)
#define del_album_music_in_slot(which_slot, music_data_value) _del_music(ALBUM_PLAYLIST_PATH, which_slot, music_data_value)

#define add_album_music_to_delete_list_in_slot(which_slot, prev_music_data_value, curr_music_data_value) _add_music(ALBUM_DELETE_LIST_PATH, which_slot, prev_music_data_value, curr_music_data_value)
#define add_album_music_to_download_list_in_slot(which_slot, prev_music_data_value, curr_music_data_value) _add_music(ALBUM_DOWNLOAD_LIST_PATH, which_slot, prev_music_data_value, curr_music_data_value)

#define init_album_playlist_hash_engine() _init_music_hash_engine(ALBUM_PLAYLIST_PATH, ALBUM_SLOT_CNT)
#define init_album_delete_list_hash_engine() _init_music_hash_engine(ALBUM_DELETE_LIST_PATH, ALBUM_SLOT_CNT)
#define init_album_download_list_hash_engine() _init_music_hash_engine(ALBUM_DOWNLOAD_LIST_PATH, ALBUM_SLOT_CNT)
/*******************************************************************/

#endif
