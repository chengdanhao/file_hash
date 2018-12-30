#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "book_hash.h"

#define HASH_INFO 1
#define HASH_DBUG 1
#define HASH_WARN 1
#define HASH_EROR 1

#if HASH_INFO
#define hash_info(fmt, ...) printf("\e[0;32m[HASH_INFO] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define hash_info(fmt, ...)
#endif

#if HASH_DBUG
#define hash_debug(fmt, ...) printf("\e[0m[HASH_DBUG] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define hash_debug(fmt, ...)
#endif

#if HASH_WARN
#define hash_warn(fmt, ...) printf("\e[0;33m[HASH_WARN] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define hash_warn(fmt, ...)
#endif

#if HASH_EROR
#define hash_error(fmt, ...) printf("\e[0;31m[HASH_EROR] [%s %d] : "fmt"\e[0m\n", __func__, __LINE__, ##__VA_ARGS__);
#else
#define hash_error(fmt, ...)
#endif

#define BOOK_NAME_LEN 14	// 最好是16的倍数减2，方便16进制工具查看
#define HASH_GROUP_CNT 3

//#define HELPER_ARRAY
#define HELPER_ARRAY_SIZE 4

typedef struct {
#ifdef HELPER_ARRAY
	char s[HELPER_ARRAY_SIZE];
#endif
	uint32_t next_offset;
	uint32_t book_code;
	uint8_t used;
	char path[BOOK_NAME_LEN + 1];
#ifdef HELPER_ARRAY
	char e[HELPER_ARRAY_SIZE];
#endif
} record_node_t;

// 该结构体不能删除，否则会出错
// 用于记录写信息，暂时没想好放什么
typedef struct {
	uint32_t reserved;
} record_property_t;

ssize_t happy_write(const char* f, int fd, void *buf, size_t count) {
	int ret = -1;
	ssize_t n_w = 0;

	if ((n_w = write(fd, buf, count)) < 0) {
		hash_error("(%s calls) write error.", f);
		goto exit;
	}

	if (n_w != count) {
		hash_error("(%s calls) write incomplete.", f);
		goto exit;
	}

	ret = 0;

exit:
	return ret;
}

ssize_t happy_read(const char* f, int fd, void *buf, size_t count) {
	int ret = -1;
	ssize_t n_r = 0;

	if ((n_r = read(fd, buf, count)) < 0) {
		hash_error("(%s calls) read error.", f);
		goto exit;
	}

	if (n_r < count) {
		hash_error("(%s calls) read incomplete.", f);
		goto exit;
	}

	ret = 0;

exit:
	return ret;
}

#define write(fd, buf, count)	happy_write(__func__, fd, buf, count)
#define read(fd, buf, count)	happy_read(__func__, fd, buf, count)

int _build_record(const char* f, char* path, uint8_t rebuild) {
	int ret = -1;
	int fd;
	int n_w = 0;
	uint8_t i = 0;
	uint8_t file_exist = 0;
	record_property_t prop;
	record_node_t node;

	if (access(path, F_OK) < 0) {
		hash_debug("(%s calls) %s not exist.", f, path);
		file_exist = 0;
	} else {
		file_exist = 1;
	}

	if (1 == file_exist && 1 == rebuild) {
		if (unlink(path) < 0) {
			hash_error("(%s calls) delete '%s' error.", f, path);
			goto exit;
		} else {
			hash_info("(%s calls) delete old record '%s' success.", f, path);
			file_exist = 0;
		}
	}

	if (0 == file_exist) {
		if ((fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
			hash_error("(%s calls) create file %s fail.", f, path);
			goto exit;
		}

		memset(&prop, 0, sizeof(record_property_t));
		prop.reserved = 0x123;
		if (write(fd, &prop, sizeof(record_property_t)) < 0) {
			hash_error("(%s calls) write prop error.", f);
			goto exit;
		}

		memset(&node, 0, sizeof(node));
#ifdef HELPER_ARRAY
		memcpy(node.s, ">>>>", HELPER_ARRAY_SIZE);
		memcpy(node.e, "<<<<", HELPER_ARRAY_SIZE);
#endif

		for (i = 0; i < HASH_GROUP_CNT; i++) {
			if (write(fd, &node, sizeof(record_node_t)) < 0) {
				hash_error("(%s calls) write node error.", f);
				goto exit;
			}
		}
	}

	ret = 0;

close_file:
	close(fd);
exit:
	return ret;
}

#define check_record(path)		_build_record(__func__, path, 0)
#define rebuild_record(path)	_build_record(__func__, path, 1)

int append_book(char* record_path, uint32_t book_code, char* path) {
	int ret = -1;
	int fd = 0;
	int n_r = 0;
	uint32_t i = 0;
	uint32_t group = 0;
	uint32_t new_node_offset = 0;
	uint32_t offset = 0;
	uint8_t more_than_one_node = 0;
	uint32_t prev_node_offset = 0;
	record_node_t prev_node;
	record_node_t curr_node;

	if (check_record(record_path) < 0) {
		hash_error("init record error.");
		goto exit;
	}

	group = book_code % HASH_GROUP_CNT;
	offset = sizeof(record_property_t) + group * sizeof(record_node_t);

	if ((fd = open(record_path, O_RDWR)) < 0) {
		hash_error("Open file %s fail.", record_path);
		goto exit;
	}

	memset(&curr_node, 0, sizeof(record_node_t));
	while (offset > 0) {
		// 拿一个节点数据，取完后文件指针不要挪动
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %d fail.", offset);
			goto exit;
		}

		if (read(fd, &curr_node, sizeof(record_node_t)) < 0) {
			hash_error("read node failed.");
			goto exit;
		}

		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek back to %d fail.", offset);
			goto exit;
		}

		if (0 == curr_node.next_offset) {
			if (1 == curr_node.used) {
				// 新节点在文件末尾插入
				if ((new_node_offset = lseek(fd, 0, SEEK_END)) < 0) {
					hash_error("prepare new node, seek to %d fail.", offset);
					goto close_file;
				}

				// 修改当前解点的next_offset值，指向新节点
				lseek(fd, offset, SEEK_SET);

				curr_node.next_offset = new_node_offset;

				if (write(fd, &curr_node, sizeof(record_node_t)) < 0) {
					hash_error("init new node error.");
					goto exit;
				}

				// 移到新节点处
				lseek(fd, 0, SEEK_END);

				hash_debug("book_code = %d, %s (0x%x) -> %s (0x%x).", book_code, curr_node.path, offset, path, new_node_offset);
			} else {
				hash_debug("book_code = %d, %s (0x%x).", book_code, curr_node.path, offset);
			}

			// 填充新节点
			curr_node.used = 1;
			curr_node.book_code = book_code;
			curr_node.next_offset = 0;
			strncpy(curr_node.path, path, BOOK_NAME_LEN);
#ifdef HELPER_ARRAY
			memcpy(curr_node.s, ">>>>", HELPER_ARRAY_SIZE);
			memcpy(curr_node.e, "<<<<", HELPER_ARRAY_SIZE);
#endif

			if (write(fd, &curr_node, sizeof(record_node_t)) < 0) {
				hash_error("init new node error.");
				goto exit;
			}

			break;
		} else {
			offset = curr_node.next_offset;
		}
	}

	ret = 0;

close_file:
	close(fd);

exit:
	return ret;	
}

void print_nodes(char* path) {
	uint8_t i = 0;
	int fd = 0;
	off_t offset = 0;
	record_node_t node;

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("Open %s fail.", path);
		goto exit;
	}

	memset(&node, 0, sizeof(record_node_t));

	for (i = 0; i < HASH_GROUP_CNT; i++) {
		if ((offset = lseek(fd, sizeof(record_property_t) + i * sizeof(record_node_t), SEEK_SET)) < 0) {
			hash_error("skip %s property failed.", path);
			goto close_file;
		}

		printf("[%d]\t<0x%.2lX> ", i, offset);

		while (offset > 0) {
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail.", offset);
				goto close_file;
			}

			if (read(fd, &node, sizeof(record_node_t)) < 0) {
				hash_error("read node failed.");
				goto close_file;
			}

			if (node.used) {
				printf("{%d :%s} -> <0x%.2X> ", node.book_code, node.path, node.next_offset);
			}

			offset = node.next_offset;
		}
		printf("\n");
	}

close_file:
	close(fd);

exit:
	return;
}

