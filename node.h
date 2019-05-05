#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>
#include "hash.h"

#define PLAYLIST_PATH "playlist"

#define MUSIC_PATH_LEN 200
#define FAVORITE_ALBUM_NAME_LEN 20
#define FAVORITE_ALBUM_CNT 3

/*
 * 后续只需要修改这个头文件就可以自定义节点数据，底层代码不用修改
 */

typedef enum {
	MUSIC_DELETE,
	MUSIC_KEEP,
	MUSIC_DO_NOTHING,
} action_t;

typedef struct {
	int reserved;
	uint32_t which_album_to_handle;
	char album_name[FAVORITE_ALBUM_CNT][FAVORITE_ALBUM_NAME_LEN];
} playlist_prop_t;

typedef struct {
	action_t delete_or_not;		// 判断歌曲是否删除
	char path[MUSIC_PATH_LEN];
} music_t;

int add_music(int hash_key, const char* music_path);
int del_music(int hash_key, const char* music_path);
void show_playlist();

/*
 * 当有多个哈希槽时，操作指定哈希槽的列表。如果只有一个哈希槽，hash_key可随意填
 *
 * reset操作将所有节点歌曲都标记为待删除（MUSIC_DELETE）。后续添加时，存在的歌曲会更新为保留（MUSIC_KEEP）。
 * 添加完所有歌曲后，让然处于待删除（MUSIC_DELETE）的歌曲，则调用clean来删除
 */
void clean_playlist(int hash_key);
void reset_playlist(int hash_key);
/********************/

// 获取播放列表属性，该属性存放在hash_property_t中
void get_playlist_prop(playlist_prop_t* playlist_prop);

// 设置播放列表属性，该属性存放在hash_property_t中
void set_playlist_prop(playlist_prop_t* playlist_prop);

// 创建/重建播放列表
void check_playlist();
void rebuild_playlist();

#endif
