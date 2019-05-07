PROJECT(hash_playlist)

SET(SRCS
  hash.c
  node.c
)

# 可执行文件
add_executable(hash_playlist main.c ${SRCS})
