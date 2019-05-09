#ifndef __MUSIC_NODE_H__
#define __MUSIC_NODE_H__

#include <stdint.h>
#include "hash.h"

#define MAX_MUSIC_PATH_LEN 200
#define MAX_PLAYLIST_NAME_LEN 20
#define MAX_PLAYLIST_CNT 3

#define STORY_PLAYLIST_PATH "story_playlist"
#define ALBUM_PLAYLIST_PATH "album_playlist"

#define STORY_SLOT_CNT 1
#define ALBUM_SLOT_CNT MAX_PLAYLIST_CNT

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

typedef enum {
	MUSIC_DELETE,
	MUSIC_KEEP,
	MUSIC_DO_NOTHING,
} action_t;

typedef struct {
	uint32_t music_cnt;
	off_t prev_offset;
	off_t next_offset;
	off_t saved_offset;	// 记录上一次播放记录
	char name[MAX_PLAYLIST_NAME_LEN];
} playlist_t;

typedef struct {
	uint32_t playlist_cnt;
	off_t saved_offset_for_all;
	uint32_t which_playlist_to_handle;
	playlist_t playlist[MAX_PLAYLIST_CNT];
} playlist_header_data_value_t;

typedef struct {
	action_t delete_or_not;		// 判断歌曲是否删除
	char path[MAX_MUSIC_PATH_LEN];
} music_data_value_t;

typedef enum {
	NEXT_MUSIC,
	PREV_MUSIC,
} direction_t;

void _get_music(const char* playlist_path, uint32_t hash_key, direction_t prev_or_next);
int _add_music(const char* playlist_path, uint32_t hash_key, const char* music_path);
int _del_music(const char* playlist_path, uint32_t hash_key, const char* music_path);
void _show_playlist(const char* playlist_path);

/*
 * 当有多个哈希槽时，操作指定哈希槽的列表。如果只有一个哈希槽，hash_key可随意填
 *
 * reset操作将所有节点歌曲都标记为待删除（MUSIC_DELETE）。后续添加时，存在的歌曲会更新为保留（MUSIC_KEEP）。
 * 添加完所有歌曲后，让然处于待删除（MUSIC_DELETE）的歌曲，则调用clean来删除
 */
void _clean_playlist(const char* playlist_path, uint32_t hash_key);
void _reset_playlist(const char* playlist_path, uint32_t hash_key);
/********************/

// 获取播放列表属性，该属性存放在hash_header_t中
void _get_playlist_header(const char* func, const int line, const char* playlist_path, playlist_header_data_value_t* header_data_value);

// 设置播放列表属性，该属性存放在hash_header_t中
void _set_playlist_header(const char* func, const int line, const char* playlist_path, playlist_header_data_value_t* header_data_value);

int _init_playlist_hash_engine(const char* path, uint32_t slot_cnt);

#define _get_prev_music(playlist_path, hash_key) _get_music(playlist_path, hash_key, PREV_MUSIC)
#define _get_next_music(playlist_path, hash_key) _get_music(playlist_path, hash_key, NEXT_MUSIC)

/********************** 故事收藏 调用这些函数 **********************/
#define get_story_prev_music(hash_key)                _get_prev_music(STORY_PLAYLIST_PATH, hash_key)
#define get_story_next_music(hash_key)                _get_next_music(STORY_PLAYLIST_PATH, hash_key)

#define add_story_music(hash_key, music_path)         _add_music(STORY_PLAYLIST_PATH, hash_key, music_path)
#define del_story_music(hash_key, music_path)         _del_music(STORY_PLAYLIST_PATH, hash_key, music_path)
#define show_story_playlist()                         _show_playlist(STORY_PLAYLIST_PATH)

#define clean_story_playlist(hash_key)                _clean_playlist(STORY_PLAYLIST_PATH, hash_key)
#define reset_story_playlist(hash_key)                _reset_playlist(STORY_PLAYLIST_PATH, hash_key)

#define get_story_playlist_header(header_data_value)  _get_playlist_header(__func__, __LINE__, STORY_PLAYLIST_PATH, header_data_value)
#define set_story_playlist_header(header_data_value)  _set_playlist_header(__func__, __LINE__, STORY_PLAYLIST_PATH, header_data_value)

#define init_story_playlist_hash_engine()             _init_playlist_hash_engine(STORY_PLAYLIST_PATH, STORY_SLOT_CNT)
/*******************************************************************/

/********************** 专辑收藏 调用这些函数 **********************/
#define get_album_prev_music(hash_key)                _get_prev_music(ALBUM_PLAYLIST_PATH, hash_key)
#define get_album_next_music(hash_key)                _get_next_music(ALBUM_PLAYLIST_PATH, hash_key)

#define add_album_music(hash_key, music_path)         _add_music(ALBUM_PLAYLIST_PATH, hash_key, music_path)
#define del_album_music(hash_key, music_path)         _del_music(ALBUM_PLAYLIST_PATH, hash_key, music_path)
#define show_album_playlist()                         _show_playlist(ALBUM_PLAYLIST_PATH)

#define clean_album_playlist(hash_key)                _clean_playlist(ALBUM_PLAYLIST_PATH, hash_key)
#define reset_album_playlist(hash_key)                _reset_playlist(ALBUM_PLAYLIST_PATH, hash_key)

#define get_album_playlist_header(header_data_value)  _get_playlist_header(__func__, __LINE__, ALBUM_PLAYLIST_PATH, header_data_value)
#define set_album_playlist_header(header_data_value)  _set_playlist_header(__func__, __LINE__, ALBUM_PLAYLIST_PATH, header_data_value)

#define init_album_playlist_hash_engine()             _init_playlist_hash_engine(ALBUM_PLAYLIST_PATH, ALBUM_SLOT_CNT)
/*******************************************************************/

#endif
