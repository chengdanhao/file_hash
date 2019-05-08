#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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

int get_header(const char* path, hash_header_data_t* output,
		int (*cb)(hash_header_data_t*, hash_header_data_t*)) {
	int fd = 0;
	int ret = -1;
	hash_header_t header;
	void* header_data_value = NULL;
	uint32_t header_data_value_size = 0;

	if ((fd = open(path, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		hash_error("create file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	header_data_value_size = header.header_data_value_size;

	if (header_data_value_size > 0) {
		if (NULL == (header_data_value = (void*)calloc(1, header_data_value_size))) {
			hash_error("calloc failed.");
			goto exit;
		}

		if (read(fd, header_data_value, header_data_value_size) < 0) {
			hash_error("read hash_header_content error : %s.", strerror(errno));
			goto close_file;
		}
	}

	header.data.value = header_data_value;

	cb(&(header.data), output);

close_file:
	close(fd);

exit:
	safe_free(header_data_value);
	return ret;

}

int set_header(const char* path, hash_header_data_t* input,
		int (*cb)(hash_header_data_t*, hash_header_data_t*)) {
	int fd = 0;
	int ret = -1;
	hash_header_t header;
	uint32_t header_data_value_size = 0;
	void *header_data_value = NULL;

	memset(&header, 0, sizeof(hash_header_t));

	if ((fd = open(path, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
		hash_error("open file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	header_data_value_size = header.header_data_value_size;

	if (lseek(fd, 0, SEEK_SET) < 0) {
		hash_error("seek to head fail : %s.", strerror(errno));
		goto close_file;
	}

	if (header_data_value_size > 0
			&& NULL == (header_data_value = (void*)calloc(1, header_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	header.data.value = header_data_value;

	cb(&(header.data), input);

	if (write(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("write header error : %s.", strerror(errno));
		goto close_file;
	}

	if (header_data_value_size > 0
			&& write(fd, header.data.value, header_data_value_size) < 0) {
		hash_error("write header.data.value error : %s.", strerror(errno));
		goto close_file;
	}

close_file:
	close(fd);

exit:
	safe_free(header_data_value);
	return ret;
}

// 获取指定哈希值或指定偏移量的节点，返回下一个节点偏移量
off_t get_node(const char* path, get_node_method_t method, uint32_t hash_key,
		off_t offset, hash_node_t* output, int (*cb)(hash_node_t*, hash_node_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	off_t curr_offset = 0;
	hash_header_t header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t hash_slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;

	memset(&header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if ((fd = open(path, O_RDONLY)) < 0) {
		hash_error("open file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	hash_slot_cnt = header.hash_slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	if (method == GET_NODE_BY_HASH_KEY) {
		group = hash_key % hash_slot_cnt;
		offset = (sizeof(hash_header_t) + header_data_value_size)\
			 + group * (sizeof(hash_node_t) + node_data_value_size);
	}

	if (NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	// 记录当前偏移量，后面会回退到该偏移量
	if ((curr_offset = lseek(fd, 0, SEEK_CUR)) < 0) {
		hash_error("seek to %ld fail : %s.", curr_offset, strerror(errno));
		goto close_file;
	}

	// 定位到指定的偏移量处
	if (lseek(fd, offset, SEEK_SET) < 0) {
		hash_error("seek to %ld fail : %s.", offset, strerror(errno));
		goto close_file;
	}

	if (read(fd, &node, sizeof(hash_node_t)) < 0) {
		hash_error("read node failed : %s.", strerror(errno));
		goto close_file;
	}

	if (read(fd, node_data_value, node_data_value_size) < 0) {
		hash_error("read node_data_value failed : %s.", strerror(errno));
		goto close_file;
	}

	// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
	node.data.value = node_data_value;

	// 在回调函数中可以返回上/下一首歌曲的偏移量
	cb(&node, output);

	// 重新指向开始时的偏移量
	if (lseek(fd, curr_offset, SEEK_SET) < 0) {
		hash_error("seek back to %ld fail : %s.", curr_offset, strerror(errno));
		goto close_file;
	}

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(node_data_value);
	return ret;
}

int add_node(const char* path, hash_node_data_t* input,
		int (*cb)(hash_node_data_t*, hash_node_data_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	off_t new_node_offset = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	hash_header_t header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t hash_slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;

	memset(&header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("open file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	hash_slot_cnt = header.hash_slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	group = input->key % hash_slot_cnt;
	offset = first_node_offset = (sizeof(hash_header_t) + header_data_value_size)\
		 + group * (sizeof(hash_node_t) + node_data_value_size);

	if (NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	do {
		/* START 拿一个节点数据，取完后文件指针不要挪动 */
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail : %s.", offset, strerror(errno));
			goto close_file;
		}

		if (read(fd, &node, sizeof(hash_node_t)) < 0) {
			hash_error("read node failed : %s.", strerror(errno));
			goto close_file;
		}

		if (read(fd, node_data_value, node_data_value_size) < 0) {
			hash_error("read node_data_value failed : %s.", strerror(errno));
			goto close_file;
		}

		// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
		node.data.value = node_data_value;

		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek back to %ld fail : %s.", offset, strerror(errno));
			goto close_file;
		}
		/* END 拿一个节点数据，取完后文件指针不要挪动 */

		/*
		 * used  next_offset  desc
		 *  0         0        首次使用第一个节点
		 *  0         1        被清空过的节点
		 *  1         0        已被使用的最后一个节点
		 */
#define MORE_ADD_NODE_INFO 0
		if (0 == node.used || first_node_offset == node.next_offset) {
			// 0 0, 首次使用第一个节点
			if (0 == node.used
					&& first_node_offset == node.next_offset
					&& first_node_offset == node.prev_offset) {
#if MORE_ADD_NODE_INFO
				hash_debug("(FIRST) <0x%lx> (0x%lx : %d ) <0x%lx>",
						node.prev_offset, offset, input->key, node.next_offset);
#endif
				node.prev_offset = node.next_offset = first_node_offset;
			}

			// 0 1, 被清空过的节点
			else if (0 == node.used && first_node_offset != node.next_offset) {
#if MORE_ADD_NODE_INFO
				hash_debug(" (USED) <0x%lx> (0x%lx : %d ) <0x%lx>",
						node.prev_offset, offset, input->key, node.next_offset);
#endif
			}

			// 1 0, 正在使用的最后一个节点
			else if (1 == node.used && first_node_offset == node.next_offset) {
				off_t curr_node_offset = offset;

				// 新节点在文件末尾插入，获取新节点偏移量
				if ((new_node_offset = lseek(fd, 0, SEEK_END)) < 0) {
					hash_error("prepare new node, seek to %ld fail : %s.",
							offset, strerror(errno));
					goto close_file;
				}

				/**** 1. START 修改当前节点的next_offset值，指向新节点 ****/
				lseek(fd, curr_node_offset, SEEK_SET);

				node.next_offset = new_node_offset;

				if (write(fd, &node, sizeof(hash_node_t)) < 0) {
					hash_error("write node error : %s.", strerror(errno));
					goto close_file;
				}
				/**** 1. END 修改当前节点的next_offset值，指向新节点 ****/

				/**** 2. START 修改头节点的prev_offset值，指向新节点 ****/
				lseek(fd, first_node_offset, SEEK_SET);

				hash_node_t first_node;

				memset(&first_node, 0, sizeof(first_node));

				if (read(fd, &first_node, sizeof(hash_node_t)) < 0) {
					hash_error("read node error : %s.", strerror(errno));
					goto close_file;
				}

				first_node.prev_offset = new_node_offset;

				// 再次回到节点开头写回
				lseek(fd, first_node_offset, SEEK_SET);

				if (write(fd, &first_node, sizeof(hash_node_t)) < 0) {
					hash_error("write node error : %s.", strerror(errno));
					goto close_file;
				}
				/**** 2. END 修改头节点的prev_offset值，指向新节点 ****/

				/**** 3. START 修改新节点的prev和next指针 ****/
				lseek(fd, 0, SEEK_END);

				node.prev_offset = curr_node_offset;
				node.next_offset = first_node_offset;
				/**** 3. END 修改新节点的prev和next指针 ****/

#if MORE_ADD_NODE_INFO
				hash_debug(" (TAIL) <0x%lx> (0x%lx : %d ) <0x%lx>",
						node.prev_offset, new_node_offset, input->key, node.next_offset);
#endif
			}
#undef MORE_ADD_NODE_INFO

			/**** 4. START 写入新节点的其他信息 ****/
			node.used = 1;

			cb(&(node.data), input);

			if (write(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("write node error : %s.", strerror(errno));
				goto close_file;
			}

			if (write(fd, node.data.value, node_data_value_size) < 0) {
				hash_error("write node_data_value error : %s.", strerror(errno));
				goto close_file;
			}
			/**** 4. END 写入新节点的其他信息 ****/

			break;
		} else {
			offset = node.next_offset;
		}
	}  while (offset != first_node_offset);

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(node_data_value);
	return ret;
}

int del_node(const char* path, hash_node_data_t* input,
		int (*cb)(hash_node_data_t*, hash_node_data_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	hash_header_t header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t hash_slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;

	memset(&header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("open file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	hash_slot_cnt = header.hash_slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	group = input->key % hash_slot_cnt;
	offset = first_node_offset = (sizeof(hash_header_t) + header_data_value_size)\
		 + group * (sizeof(hash_node_t) + node_data_value_size);

	if (NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	do {
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail : %s.", offset, strerror(errno));
			goto close_file;
		}

		if (read(fd, &node, sizeof(hash_node_t)) < 0) {
			hash_error("read node failed : %s.", strerror(errno));
			goto close_file;
		}

		if (read(fd, node_data_value, node_data_value_size) < 0) {
			hash_error("read node_data_value failed : %s.", strerror(errno));
			goto close_file;
		}

		// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
		node.data.value = node_data_value;

		// 比较的同时，清空node.data中的相关数据
		if (0 == cb(&(node.data), input)) {
			node.used = 0;

			// 移到节点起始位置
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek back to %ld fail : %s.", offset, strerror(errno));
				goto close_file;
			}

			if (write(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("del node error : %s.", strerror(errno));
				goto close_file;
			}

			if (write(fd, node.data.value, node_data_value_size) < 0) {
				hash_error("del node_data_value error : %s.", strerror(errno));
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
	safe_free(node_data_value);
	return ret;
}

// traverse_type 为 TRAVERSE_ALL 时，hash_key可随意填写
uint8_t traverse_nodes(const char* path, traverse_type_t traverse_type, uint32_t hash_key,
		print_t print, hash_node_data_t* input, traverse_action_t (*cb)(hash_node_t*, hash_node_data_t*)) {
	traverse_action_t action = TRAVERSE_ACTION_DO_NOTHING;
	uint8_t i = 0;
	int fd = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	hash_header_t hash_header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t hash_slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;
	uint8_t break_or_not = 0;
	static uint8_t s_first_node = 1;

	memset(&hash_header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("open %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &hash_header, sizeof(hash_header_t)) < 0) {
		hash_error("read hash_header error : %s.", strerror(errno));
		goto close_file;
	}

	hash_slot_cnt = hash_header.hash_slot_cnt;
	header_data_value_size = hash_header.header_data_value_size;
	node_data_value_size = hash_header.node_data_value_size;

	if (NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	for (i = 0; i < hash_slot_cnt; i++) {

		// TODO: hash_key和i的关系不一定可以直接比较，后续版本需要完善
		if (TRAVERSE_SPECIFIC_HASH_KEY == traverse_type && i != (hash_key % hash_slot_cnt)) {
			continue;
		}

		offset = first_node_offset = (sizeof(hash_header_t) + header_data_value_size)\
			 + i * (sizeof(hash_node_t) + node_data_value_size);

		s_first_node = 1;
		if (lseek(fd, first_node_offset, SEEK_SET) < 0) {
			hash_error("skip %s hash header failed : %s.", path, strerror(errno));
			goto close_file;
		}

		if (WITH_PRINT == print) { printf("[%d]\t", i); }

		do {
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("read node failed : %s.", strerror(errno));
				goto close_file;
			}

			if (read(fd, node_data_value, node_data_value_size) < 0) {
				hash_error("read node_data_value failed : %s.", strerror(errno));
				goto close_file;
			}

			node.data.value = node_data_value;

			if (s_first_node) {
				s_first_node = 0;
			} else {
				if (WITH_PRINT == print) { printf(" --- "); }
			}

			if (WITH_PRINT == print) { printf("<0x%lX> ( 0x%lX : ", node.prev_offset, offset); }

			action = cb(&node, input);

			if (WITH_PRINT == print) { printf(" ) <0x%lX>", node.next_offset); }

			if (TRAVERSE_ACTION_UPDATE & action) {
				// 跳回到节点头部
				if (lseek(fd, offset, SEEK_SET) < 0) {
					hash_error("seek to %ld fail : %s.", offset, strerror(errno));
					goto close_file;
				}

				if (write(fd, &node, sizeof(hash_node_t)) < 0) {
					hash_error("del node error : %s.", strerror(errno));
					perror("delete");
					goto close_file;
				}

				if (write(fd, node.data.value, node_data_value_size) < 0) {
					hash_error("del node_data_value error : %s.", strerror(errno));
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
	safe_free(node_data_value);
	return break_or_not;
}

int init_hash_engine(const char* path, init_method_t rebuild,
		int hash_slot_cnt, int node_data_value_size, int header_data_value_size) {
	int ret = -1;
	int fd = 0;
	uint32_t i = 0;
	uint8_t file_exist = 0;
	hash_header_t header;
	void* header_data_value = NULL;
	hash_node_t node;
	void* node_data_value = NULL;
	off_t offset = 0;

	hash_info("path = %s, rebuild = %d, "
			"hash_slot_cnt = %d, node_data_value_size = %d, header_data_value_size = %d.",
			path, rebuild, hash_slot_cnt, node_data_value_size, header_data_value_size);

	memset(&header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if (access(path, F_OK) < 0) {
		hash_debug("%s not exist.", path);
		file_exist = 0;
	} else {
		file_exist = 1;
	}

	if (1 == file_exist && 1 == rebuild) {
		if (unlink(path) < 0) {
			hash_error("delete '%s' error : %s.", path, strerror(errno));
			goto exit;
		} else {
			hash_info("delete old hash file '%s' success.", path);
			file_exist = 0;
		}
	}

	if (0 == file_exist) {
		if ((fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
			hash_error("create file %s fail : %s.", path, strerror(errno));
			goto exit;
		}

		// 先写入头部信息
		if (header_data_value_size > 0
				&& NULL == (header_data_value = (void*)calloc(1, header_data_value_size))) {
			hash_error("calloc failed.");
			goto exit;
		}

		header.hash_slot_cnt = hash_slot_cnt;
		header.header_data_value_size = header_data_value_size;
		header.node_data_value_size = node_data_value_size;
		header.data.value = header_data_value;

		if (write(fd, &header, sizeof(hash_header_t)) < 0) {
			hash_error("write header error : %s.", strerror(errno));
			goto close_file;
		}

		if (header_data_value_size > 0
				&& write(fd, header.data.value, header_data_value_size) < 0) {
			hash_error("write header.data.value error : %s.", strerror(errno));
			goto close_file;
		}

		if (NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
			hash_error("calloc failed.");
			goto close_file;
		}

		node.data.value = node_data_value;

		for (i = 0; i < hash_slot_cnt; i++) {
			offset = (sizeof(hash_header_t) + header_data_value_size) + i * (sizeof(hash_node_t) + node_data_value_size);
			node.prev_offset = node.next_offset = offset;

			if (write(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("init node error : %s.", strerror(errno));
				goto close_file;
			}

			if (write(fd, node.data.value, node_data_value_size) < 0) {
				hash_error("init node.data.value error : %s.", strerror(errno));
				goto close_file;
			}
		}
	}

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(header_data_value);
	safe_free(node_data_value);
	return ret;
}
