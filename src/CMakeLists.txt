PROJECT(file_hash)

SET(SRCS
  hash_layer/hash.c
  alarm_tone_list/alarm_tone_node.c
  alarm_tone_list/test_alarm_tone_list.c
  music_playlist/music_node.c
  music_playlist/test_music_playlist.c
  to_be_download_list/tbd_node.c
  to_be_download_list/test_tbd_list.c
)

# 可执行文件
add_executable(file_hash main.c ${SRCS})
