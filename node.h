#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>
#include "hash.h"

#define PLAYLIST_PATH "playlist"

#define MUSIC_PATH_LEN 200

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

typedef enum {
	MUSIC_DELETE,
	MUSIC_KEEP,
	MUSIC_DO_NOTHING,
} action_t;

typedef struct {
	uint32_t which_album_to_handle;
	uint32_t reserved;
} playlist_prop_t;

typedef struct {
	action_t delete_or_not;		// 判断歌曲是否删除
	char path[MUSIC_PATH_LEN];
} music_t;

int add_music(const char* music_path);
int del_music(const char* music_path);
void show_playlist();
void clean_playlist();
void reset_playlist();
off_t is_music_exist(const char* music_path);
void get_playlist_prop();
void set_playlist_prop(playlist_prop_t* playlist_prop);

#endif
