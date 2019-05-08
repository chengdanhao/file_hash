# file_hash

[![Build Status](https://travis-ci.org/chengdanhao/file_hash.svg?branch=master)](https://travis-ci.org/chengdanhao/file_hash)

**项目来源于一个客户需求，无敏感信息。**

文件形式存储双链表，根据用户指定的哈希值，将内容散列到不同的哈希槽当中，每个哈希槽的节点以链式方式存储，支持增、删、查、改。

已有支持歌曲播放列表和闹钟链表两个demo。


# 代码结构
```
.
├── CMakeLists.txt
├── inc
│   ├── alarm_tone_list
│   │   └── alarm_tone_node.h
│   ├── CMakeLists.txt
│   ├── hash_layer
│   │   └── hash.h
│   └── music_playlist
│       └── music_node.h
├── README.md
└── src
    ├── alarm_tone_list
    │   ├── alarm_tone_node.c
    │   └── test_alarm_tone_list.c
    ├── CMakeLists.txt
    ├── hash_layer
    │   └── hash.c
    ├── main.c
    └── music_playlist
        ├── music_node.c
        └── test_music_playlist.c
```
- 业务层：alarm_tone_list 和 music_playlist
- 抽象层：hash_layer ，可以参考 alarm_tone_list 和 music_playlist 自定义的节点内容。

# 编译方法
1. 进入工程根目录
2. mkdir build
3. cd build && cmake ..
4. make
5. bin/file_hash
