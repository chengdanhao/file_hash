# hash_playlist

**项目来源于一个客户需求，无敏感信息。**

文件形式存储歌曲播放列表，根据用户指定的哈希值，将歌曲散列到不同的哈希槽当中，每个哈希槽的节点以链式方式存储，支持歌曲增、删、查、改。


# 代码结构
```
.
├── CMakeLists.txt
├── inc
│   ├── CMakeLists.txt
│   ├── hash.h
│   └── node.h
└── src
    ├── CMakeLists.txt
    ├── hash.c
    ├── main.c
    └── node.c
```
- 业务层：main.c
- 表现层：node.c 和 node.h
- 存储层：hash.c 和 hash.h ，这一层已经抽象出来，可以参考 node.c 和 node.h 自定义的节点内容。

# 编译方法
1. 进入工程根目录
2. mkdir build
3. cd build && cmake ..
4. make
5. bin/hash_playlist
