#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "hash.h"

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

static int s_hash_slot_cnt;
static int s_hash_value_size;
static int s_hash_property_size;

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

int get_hash_prop(char* path, hash_property_t* output, int (*cb)(hash_property_t*, hash_property_t*)) {
	int fd = 0;
	int ret = -1;
	int curr_offset = 0;
	hash_property_t prop;
	void* prop_content = NULL;

	if (NULL == (prop_content = (void*)calloc(1, s_hash_property_size))) {
		hash_error("malloc failed.");
		goto exit;
	}

	if ((fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		hash_error("create file %s fail.", path);
		goto exit;
	}

	if ((curr_offset = lseek(fd, 0, SEEK_CUR)) < 0) {
		hash_error("seek to %d fail.", curr_offset);
		goto close_file;
	}

	if (lseek(fd, 0, SEEK_SET) < 0) {
		hash_error("seek to head fail.");
		goto close_file;
	}

	if (read(fd, &prop, sizeof(hash_property_t)) < 0) {
		hash_error("read hash_prop error.");
		goto close_file;
	}

	if (read(fd, prop_content, s_hash_property_size) < 0) {
		hash_error("read hash_prop_content error.");
		goto close_file;
	}

	prop.prop = prop_content;

	cb(&prop, output);

	if (lseek(fd, curr_offset, SEEK_SET) < 0) {
		hash_error("seek back to %d fail.", curr_offset);
		goto close_file;
	}

close_file:
	close(fd);

exit:
	safe_free(prop_content);
	return ret;

}

int set_hash_prop(char* path, hash_property_t* input, int (*cb)(hash_property_t*, hash_property_t*)) {
	int fd = 0;
	int ret = -1;
	int curr_offset = 0;
	hash_property_t prop;
	void *prop_content = NULL;

	memset(&prop, 0, sizeof(hash_property_t));

	if (NULL == (prop_content = (void*)calloc(1, s_hash_property_size))) {
		hash_error("malloc failed.");
		goto exit;
	}

	if ((fd = open(path, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		hash_error("open file %s fail.", path);
		goto exit;
	}

	if ((curr_offset = lseek(fd, 0, SEEK_CUR)) < 0) {
		hash_error("seek to %d fail.", curr_offset);
		goto close_file;
	}

	if (lseek(fd, 0, SEEK_SET) < 0) {
		hash_error("seek to head fail.");
		goto close_file;
	}

	prop.prop = prop_content;

	cb(&prop, input);

	if (write(fd, &prop, sizeof(hash_property_t)) < 0) {
		hash_error("write prop error.");
		goto close_file;
	}

	if (write(fd, prop.prop, s_hash_property_size) < 0) {
		hash_error("write prop_content error.");
		goto close_file;
	}

	if (lseek(fd, curr_offset, SEEK_SET) < 0) {
		hash_error("seek back to %d fail.", curr_offset);
		goto close_file;
	}

close_file:
	close(fd);

exit:
	safe_free(prop_content);
	return ret;
}

int _build_hash_file(const char* f, char* path, uint8_t rebuild) {
	int ret = -1;
	int fd = 0;
	uint32_t i = 0;
	uint8_t file_exist = 0;
	hash_property_t prop;
	void* prop_content = NULL;
	file_node_t node;
	void* hash_value = NULL;
	off_t offset = 0;

	memset(&prop, 0, sizeof(hash_property_t));
	memset(&node, 0, sizeof(file_node_t));

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

		if (NULL == (prop_content = (void*)calloc(1, s_hash_property_size))) {
			hash_error("malloc failed.");
			goto exit;
		}

		if (write(fd, &prop, sizeof(hash_property_t)) < 0) {
			hash_error("(%s calls) write prop error.", f);
			goto close_file;
		}

		if (write(fd, prop_content, s_hash_property_size) < 0) {
			hash_error("(%s calls) write prop error.", f);
			goto close_file;
		}

		if (NULL == (hash_value = (void*)calloc(1, s_hash_value_size))) {
			hash_error("(%s calls) malloc failed.", f);
			goto close_file;
		}

		for (i = 0; i < s_hash_slot_cnt; i++) {
			offset = (sizeof(hash_property_t) + s_hash_property_size) + i * (sizeof(file_node_t) + s_hash_value_size);
			node.prev_offset = node.next_offset = offset;
			if (write(fd, &node, sizeof(file_node_t)) < 0) {
				hash_error("(%s calls) init node error.", f);
				goto close_file;
			}

			if (write(fd, hash_value, s_hash_value_size) < 0) {
				hash_error("(%s calls) init hash_value error.", f);
				goto close_file;
			}
		}
	}

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(prop_content);
	safe_free(hash_value);
	return ret;
}

int add_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	off_t new_node_offset = 0;
	off_t offset = 0;
	off_t prev_offset = 0;
	off_t first_node_offset = 0;
	file_node_t node;
	void* hash_value = NULL;

	memset(&node, 0, sizeof(file_node_t));

	group = input->hash_key % s_hash_slot_cnt;
	prev_offset = offset = first_node_offset = (sizeof(hash_property_t) + s_hash_property_size)\
		+ group * (sizeof(file_node_t) + s_hash_value_size);

	if (NULL == (hash_value = (void*)calloc(1, s_hash_value_size))) {
		hash_error("malloc failed.");
		goto exit;
	}

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("Open file %s fail.", path);
		goto exit;
	}

	do {
		/* START 拿一个节点数据，取完后文件指针不要挪动 */
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail.", offset);
			goto close_file;
		}

		if (read(fd, &node, sizeof(file_node_t)) < 0) {
			hash_error("read node failed.");
			goto close_file;
		}

		if (read(fd, hash_value, s_hash_value_size) < 0) {
			hash_error("read hash_value failed.");
			goto close_file;
		}

		node.data.hash_value = hash_value;	// 建立关联，方便后面使用

		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek back to %ld fail.", offset);
			goto close_file;
		}
		/* END 拿一个节点数据，取完后文件指针不要挪动 */

		/*
		used  next_offset  desc
		 0		  0 	   首次使用第一个节点
		 0		  1 	   被清空过的节点
		 1		  0 	   已被使用的最后一个节点
		*/
#define MORE_ADD_NODE_INFO 0
		if (0 == node.used || first_node_offset == node.next_offset) {
			// 0 0, 首次使用第一个节点
			if (0 == node.used && first_node_offset == node.next_offset) {
#if MORE_ADD_NODE_INFO
				hash_debug("(FIRST) <0x%lx> { %d }", offset, input->hash_key);
#endif
				node.next_offset = first_node_offset;
			}

			// 0 1, 被清空过的节点
			else if (0 == node.used && node.next_offset > 0) {
#if MORE_ADD_NODE_INFO
				hash_debug(" (USED) <0x%lx> { %d } -> <0x%lx>",
					offset, input->hash_key, node.next_offset);
#endif
			}

			// 1 0, 已被使用的最后一个节点
			else if (1 == node.used && first_node_offset == node.next_offset) {
				// 新节点在文件末尾插入
				if ((new_node_offset = lseek(fd, 0, SEEK_END)) < 0) {
					hash_error("prepare new node, seek to %ld fail.", offset);
					goto close_file;
				}

				// 修改当前解点的next_offset值，指向新节点
				lseek(fd, offset, SEEK_SET);

				node.prev_offset = prev_offset;
				node.next_offset = new_node_offset;

				if (write(fd, &node, sizeof(file_node_t)) < 0) {
					hash_error("write node error.");
					goto close_file;
				}

				// 移到新节点处
				lseek(fd, 0, SEEK_END);

				node.next_offset = first_node_offset;

#if MORE_ADD_NODE_INFO
				hash_debug(" (TAIL) <0x%lx> { %d } -> <0x%lx> { %d }",
					offset, node.data.hash_key, new_node_offset, input->hash_key);
#endif
			}
#undef MORE_ADD_NODE_INFO

			node.used = 1;

			cb(&(node.data), input);

			if (write(fd, &node, sizeof(file_node_t)) < 0) {
				hash_error("write node error.");
				goto close_file;
			}

			if (write(fd, node.data.hash_value, s_hash_value_size) < 0) {
				hash_error("write hash_value error.");
				goto close_file;
			}

			break;
		} else {
			prev_offset = offset;
			offset = node.next_offset;
		}
	}  while (offset != first_node_offset);

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(hash_value);
	return ret;	
}

int del_node(char* path, node_data_t* input, int (*cb)(node_data_t*, node_data_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	file_node_t node;
	void* hash_value = NULL;

	memset(&node, 0, sizeof(file_node_t));

	if (access(path, F_OK) < 0) {
		hash_debug("%s not exist.", path);
		goto exit;
	}

	group = input->hash_key % s_hash_slot_cnt;
	offset = first_node_offset = (sizeof(hash_property_t) + s_hash_property_size)\
		+ group * (sizeof(file_node_t) + s_hash_value_size);

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("Open file %s fail.", path);
		goto exit;
	}

	if (NULL == (hash_value = (void*)calloc(1, s_hash_value_size))) {
		hash_error("malloc failed.");
		goto exit;
	}

	do {
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail.", offset);
			goto close_file;
		}

		if (read(fd, &node, sizeof(file_node_t)) < 0) {
			hash_error("read node failed.");
			goto close_file;
		}

		if (read(fd, hash_value, s_hash_value_size) < 0) {
			hash_error("read hash_value failed.");
			goto close_file;
		}

		node.data.hash_value = hash_value;	// 建立关联，方便后面使用

		// 比较的同时，清空node.data中的相关数据
		if (0 == cb(&(node.data), input)) {
			node.used = 0;

			// 移到节点起始位置
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek back to %ld fail.", offset);
				goto close_file;
			}

			if (write(fd, &node, sizeof(file_node_t)) < 0) {
				hash_error("del node error.");
				perror("delete");
				goto close_file;
			}

			if (write(fd, node.data.hash_value, s_hash_value_size) < 0) {
				hash_error("del hash_value error.");
				goto close_file;
			}

			ret = 0;

			break;
		}
		
		offset = node.next_offset;
	} while (offset != first_node_offset);

close_file:
	close(fd);

exit:
	safe_free(hash_value);
	return ret;
}

// traverse_type 为 TRAVERSE_ALL 时，hash_key可随意填写
uint8_t traverse_nodes(char* path, traverse_type_t traverse_type, uint32_t hash_key, print_t print, node_data_t* input, traverse_action_t (*cb)(file_node_t*, node_data_t*)) {
	traverse_action_t action = TRAVERSE_ACTION_DO_NOTHING;
	uint8_t i = 0;
	int fd = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	file_node_t node;
	void* hash_value = NULL;
	uint8_t break_or_not = 0;
	static uint8_t s_first_node = 1;

	memset(&node, 0, sizeof(file_node_t));

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("Open %s fail.", path);
		goto exit;
	}

	if (NULL == (hash_value = (void*)calloc(1, s_hash_value_size))) {
		hash_error("malloc failed.");
		goto exit;
	}

	for (i = 0; i < s_hash_slot_cnt; i++) {

		// TODO: hash_key和i的关系不一定可以直接比较，后续版本需要完善
		if (TRAVERSE_SPECIFIC_HASH_KEY == traverse_type && i != (hash_key % s_hash_slot_cnt)) {
			continue;
		}

		offset = first_node_offset = (sizeof(hash_property_t) + s_hash_property_size)\
			+ i * (sizeof(file_node_t) + s_hash_value_size);

		s_first_node = 1;
		if (lseek(fd, first_node_offset, SEEK_SET) < 0) {
			hash_error("skip %s property failed.", path);
			goto close_file;
		}

		if (WITH_PRINT == print) { printf("[%d]\t", i); }

		do {
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail.", offset);
				goto close_file;
			}

			if (read(fd, &node, sizeof(file_node_t)) < 0) {
				hash_error("read node failed.");
				goto close_file;
			}

			if (read(fd, hash_value, s_hash_value_size) < 0) {
				hash_error("read hash_value failed.");
				goto close_file;
			}

			node.data.hash_value = hash_value;

			if (s_first_node) {
				s_first_node = 0;
			} else {
				if (WITH_PRINT == print) { printf(" -> "); }
			}

			if (WITH_PRINT == print) { printf("<0x%.2lX> ", offset); }

			action = cb(&node, input);

			if (TRAVERSE_ACTION_UPDATE & action) {
				// 跳回到节点头部
				if (lseek(fd, offset, SEEK_SET) < 0) {
					hash_error("seek to %ld fail.", offset);
					goto close_file;
				}

				if (write(fd, &node, sizeof(file_node_t)) < 0) {
					hash_error("del node error.");
					perror("delete");
					goto close_file;
				}

				if (write(fd, node.data.hash_value, s_hash_value_size) < 0) {
					hash_error("del hash_value error.");
					goto close_file;
				}
			}

			if (TRAVERSE_ACTION_BREAK & action) {
				break_or_not = 1;
				goto close_file;
			}

			offset = node.next_offset;
		} while (offset != first_node_offset);

		if (WITH_PRINT == print) { printf("\n"); }
	}

close_file:
	close(fd);

exit:
	safe_free(hash_value);
	return break_or_not;
}

void init_hash_engine(int hash_slot_cnt, int hash_value_size, int hash_property_size) {
	s_hash_slot_cnt = hash_slot_cnt;
	s_hash_value_size = hash_value_size;
	s_hash_property_size = hash_property_size;
	hash_info("hash_slot_cnt = %d, hash_value_size = %d, hash_property_size = %d.",
		hash_slot_cnt, hash_value_size, hash_property_size);
}
