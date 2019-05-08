#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>
#include "hash.h"

#define PLAYLIST_PATH "playlist"

#define MAX_MUSIC_PATH_LEN 200
#define MAX_PLAYLIST_NAME_LEN 20
#define MAX_PLAYLIST_CNT 3

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
} playlist_header_t;

typedef struct {
	action_t delete_or_not;		// 判断歌曲是否删除
	char path[MAX_MUSIC_PATH_LEN];
} music_value_t;

typedef enum {
	NEXT_MUSIC,
	PREV_MUSIC,
} direction_t;

void get_prev_music(uint32_t hash_key);
void get_next_music(uint32_t hash_key);

int add_music(uint32_t hash_key, const char* music_path);
int del_music(uint32_t hash_key, const char* music_path);
void show_playlist();

/*
 * 当有多个哈希槽时，操作指定哈希槽的列表。如果只有一个哈希槽，hash_key可随意填
 *
 * reset操作将所有节点歌曲都标记为待删除（MUSIC_DELETE）。后续添加时，存在的歌曲会更新为保留（MUSIC_KEEP）。
 * 添加完所有歌曲后，让然处于待删除（MUSIC_DELETE）的歌曲，则调用clean来删除
 */
void clean_playlist(uint32_t hash_key);
void reset_playlist(uint32_t hash_key);
/********************/

void _get_playlist_header(const char* func, const int line, playlist_header_t* playlist_header);
void _set_playlist_header(const char* func, const int line, playlist_header_t* playlist_header);
// 获取播放列表属性，该属性存放在hash_header_t中
#define get_playlist_header(playlist_header) _get_playlist_header(__func__, __LINE__, playlist_header)

// 设置播放列表属性，该属性存放在hash_header_t中
#define set_playlist_header(playlist_header) _set_playlist_header(__func__, __LINE__, playlist_header)

// 创建/重建播放列表
void check_playlist();
void rebuild_playlist();

#endif
