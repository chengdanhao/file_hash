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

ssize_t happy_write(const char* func, const int line, int fd, void *buf, size_t count) {
	int ret = -1;
	ssize_t n_w = 0;

	if ((n_w = write(fd, buf, count)) < 0) {
		hash_error("(%s : %d calls) write error.", func, line);
		goto exit;
	}

	if (n_w != count) {
		hash_error("(%s : %d calls) write incomplete, n_w = %ld, count = %ld.", func, line, n_w, count);
		goto exit;
	}

	ret = 0;

exit:
	return ret;
}

ssize_t happy_read(const char* func, const int line, int fd, void *buf, size_t count) {
	int ret = -1;
	ssize_t n_r = 0;

	if ((n_r = read(fd, buf, count)) < 0) {
		hash_error("(%s : %d calls) read error.", func, line);
		goto exit;
	}

	if (n_r < count) {
		hash_error("(%s : %d calls) read incomplete, n_r = %ld, count = %ld.", func, line, n_r, count);
		goto exit;
	}

	ret = 0;

exit:
	return ret;
}

#define write(fd, buf, count)	happy_write(__func__, __LINE__, fd, buf, count)
#define read(fd, buf, count)	happy_read(__func__, __LINE__, fd, buf, count)

int get_header(const char* path, hash_header_data_t* output_header_data,
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

	cb(&(header.data), output_header_data);

close_file:
	close(fd);

exit:
	safe_free(header_data_value);
	return ret;
}

int set_header(const char* path, hash_header_data_t* input_header_data,
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

	cb(&(header.data), input_header_data);

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

int get_node(const char* path, get_node_method_t method, uint32_t hash_key,
		off_t offset, hash_node_t* output_node, int (*cb)(hash_node_t*, hash_node_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t group = 0;
	hash_header_t header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t slot_cnt = 0;
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

	slot_cnt = header.slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	if (method == GET_NODE_BY_HASH_SLOT) {
		group = hash_key % slot_cnt;
		offset = (sizeof(hash_header_t) + header_data_value_size)\
			 + group * (sizeof(hash_node_t) + node_data_value_size);
	}

	if (node_data_value_size > 0
			&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
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

	if (node_data_value_size > 0
			&& read(fd, node_data_value, node_data_value_size) < 0) {
		hash_error("read node_data_value failed : %s.", strerror(errno));
		goto close_file;
	}

	// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
	node.data.value = node_data_value;

	// 在回调函数中可以返回上/下一首歌曲的偏移量
	cb(&node, output_node);

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(node_data_value);
	return ret;
}

int set_node(const char* path, get_node_method_t method, uint32_t hash_key,
		off_t offset, hash_node_t* input_node, int (*cb)(hash_node_t*, hash_node_t*)) {
	int ret = -1;
	int fd = 0;
	hash_header_t header;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t group = 0;
	uint32_t slot_cnt = 0;
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

	slot_cnt = header.slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	if (method == GET_NODE_BY_HASH_SLOT) {
		group = hash_key % slot_cnt;
		offset = (sizeof(hash_header_t) + header_data_value_size)\
			 + group * (sizeof(hash_node_t) + node_data_value_size);
	}

	node_data_value_size = header.node_data_value_size;

	if (node_data_value_size > 0
			&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
	node.data.value = node_data_value;

	// 定位到指定的偏移量处
	if (lseek(fd, offset, SEEK_SET) < 0) {
		hash_error("seek to %ld fail : %s.", offset, strerror(errno));
		goto close_file;
	}

	// 在回调函数中可以返回上/下一首歌曲的偏移量
	cb(&node, input_node);

	// 再次定位到节点起始位置
	if (lseek(fd, offset, SEEK_SET) < 0) {
		hash_error("seek back to %ld fail : %s.", offset, strerror(errno));
		goto close_file;
	}

	if (write(fd, &node, sizeof(hash_node_t)) < 0) {
		hash_error("write node error : %s.", strerror(errno));
		goto close_file;
	}

	if (node_data_value_size > 0
			&& write(fd, node.data.value, node_data_value_size) < 0) {
		hash_error("write node.data.value error : %s.", strerror(errno));
		goto close_file;
	}

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(node.data.value);
	return ret;
}

#define MORE_ADD_NODE_INFO 0
int add_node(const char* path,
		hash_node_data_t* input_prev_node_data, hash_node_data_t* input_curr_node_data,
		int (*cb)(hash_node_data_t*, hash_node_data_t*)) {
	int ret = -1;
	int fd = 0;
	bool find_prev_node = false;
	bool is_first_node = false;
	uint32_t which_slot = 0;
	off_t physic_offset = 0;
	off_t first_physic_node_offset = 0;
	off_t first_logic_node_offset = 0;
	off_t prev_logic_node_offset = 0;
	off_t next_logic_node_offset = 0;
	off_t new_physic_node_offset = 0;
	hash_header_t header;
	slot_info_t* slots = NULL;
	slot_info_t slot_info;
	hash_node_t first_physic_node;
	hash_node_t curr_physic_node;
	hash_node_t prev_logic_node;
	hash_node_t next_logic_node;
	void* node_data_value = NULL;
	uint32_t slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;
	void *addr = NULL;	// 防止在memcpy中，文件中保存的上一次指针值覆盖了当前正在运行的指针

	memset(&header, 0, sizeof(hash_header_t));
	memset(&slot_info, 0, sizeof(slot_info));
	memset(&first_physic_node, 0, sizeof(hash_node_t));
	memset(&curr_physic_node, 0, sizeof(hash_node_t));
	memset(&prev_logic_node, 0, sizeof(hash_node_t));
	memset(&next_logic_node, 0, sizeof(hash_node_t));
	is_first_node = input_curr_node_data->is_first_node;

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("open file %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	slot_cnt = header.slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	if (NULL == (slots = (void*)calloc(slot_cnt, sizeof(slot_info_t)))) {
		hash_error("calloc failed.");
		goto exit;
	}

	if (read(fd, slots, slot_cnt * sizeof(slot_info_t)) < 0) {
		hash_error("read slot_info error : %s.", strerror(errno));
		goto close_file;
	}

	header.slots = slots;

	which_slot = input_curr_node_data->key % slot_cnt;
	first_physic_node_offset = sizeof(hash_header_t) + slot_cnt * sizeof(slot_info_t) + header_data_value_size\
		 + which_slot * (sizeof(hash_node_t) + node_data_value_size);
	first_logic_node_offset = slots[which_slot].first_logic_node_offset;

	if (node_data_value_size > 0
			&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	// 读取第一个逻辑节点
	if (lseek(fd, first_logic_node_offset, SEEK_SET) < 0) {
		hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
		goto close_file;
	}

	if (read(fd, &prev_logic_node, sizeof(hash_node_t)) < 0) {
		hash_error("read curr_physic_node failed : %s.", strerror(errno));
		goto close_file;
	}

	if (true == is_first_node) {
#if MORE_ADD_NODE_INFO
		hash_debug("first node, clear list.");
#endif
	}

	physic_offset = first_physic_node_offset;
	do {
		// 如果是第一个节点，需要把所有节点清空
		if (true == is_first_node) {
			if (lseek(fd, physic_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
				hash_error("read curr_physic_node failed : %s.", strerror(errno));
				goto close_file;
			}

			if (0 == curr_physic_node.used) {
				goto next_loop;
			}

			// 对于已使用的节点，清空。
			// 重新回到节点开头
			if (lseek(fd, physic_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
				goto close_file;
			}

			curr_physic_node.used = 0;
			memset(node_data_value, 0, node_data_value_size);

			if (write(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
				hash_error("write curr_physic_node error : %s.", strerror(errno));
				goto close_file;
			}

			if (node_data_value_size > 0
					&& write(fd, node_data_value, node_data_value_size) < 0) {
				hash_error("write node_data_value error : %s.", strerror(errno));
				goto close_file;
			}
		}

		// 不是第一个节点，先找到上一个节点的位置
		else {
			if (lseek(fd, physic_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
				hash_error("read curr_physic_node failed : %s.", strerror(errno));
				goto close_file;
			}

			// 未使用的节点直接跳过
			if (0 == curr_physic_node.used) {
				//hash_debug("continue");
				goto next_loop;
			}

			if (node_data_value_size > 0
					&& read(fd, node_data_value, node_data_value_size) < 0) {
				hash_error("read node_data_value error : %s.", strerror(errno));
				goto close_file;
			}

			curr_physic_node.data.value = node_data_value;
			if (0 == cb(&(curr_physic_node.data), input_prev_node_data)) {
				find_prev_node = true;
				prev_logic_node = curr_physic_node;
				prev_logic_node_offset = physic_offset;
#if MORE_ADD_NODE_INFO
				hash_debug("prev node at 0x%lX.", prev_logic_node_offset);
#endif
				break;
			}
		}

next_loop:
		physic_offset = curr_physic_node.offsets.physic_next;
	} while (physic_offset != first_physic_node_offset);

	if (true == is_first_node) {
		header.slots[which_slot].node_cnt = 0;
#if MORE_ADD_NODE_INFO
		hash_debug("finish clear list.");
#endif
	} else if (false == find_prev_node) {
		if (header.slots[which_slot].node_cnt > 0) {
			hash_error("didn't find prev node.");
			goto close_file;
		} else {
			hash_warn("no node in this slot, add first node.");
			is_first_node = true;
		}
	}

	do {
		/* START 拿一个节点数据，取完后文件指针不要挪动 */
		if (lseek(fd, physic_offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
			goto close_file;
		}

		if (read(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
			hash_error("read curr_physic_node failed : %s.", strerror(errno));
			goto close_file;
		}

		if (node_data_value_size > 0
				&& read(fd, node_data_value, node_data_value_size) < 0) {
			hash_error("read node_data_value failed : %s.", strerror(errno));
			goto close_file;
		}

		// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
		curr_physic_node.data.value = node_data_value;

		if (lseek(fd, physic_offset, SEEK_SET) < 0) {
			hash_error("seek back to %ld fail : %s.", physic_offset, strerror(errno));
			goto close_file;
		}
		/* END 拿一个节点数据，取完后文件指针不要挪动 */

		/*
		 * used  next_offset  desc
		 *  0         0        首次使用第一个节点
		 *  0         1        被清空过的节点
		 *  1         0        已被使用的最后一个节点
		 */

		if (0 == curr_physic_node.used || first_physic_node_offset == curr_physic_node.offsets.physic_next) {
			// 0 0, 首次使用第一个节点
			if (0 == curr_physic_node.used
					&& first_physic_node_offset == curr_physic_node.offsets.physic_next
					&& first_physic_node_offset == curr_physic_node.offsets.physic_prev) {
#if MORE_ADD_NODE_INFO
				hash_debug("(FIRST) <0x%lX> (0x%lX : %d) <0x%lX>",
						curr_physic_node.offsets.physic_prev, physic_offset, input_curr_node_data->key, curr_physic_node.offsets.physic_next);
#endif
				curr_physic_node.offsets.physic_prev = curr_physic_node.offsets.physic_next = first_physic_node_offset;
				new_physic_node_offset = physic_offset;
			}

			// 0 1, 被清空过的节点
			else if (0 == curr_physic_node.used) {
#if MORE_ADD_NODE_INFO
				hash_debug(" (USED) <0x%lX> (0x%lX : %d) <0x%lX>",
						curr_physic_node.offsets.physic_prev, physic_offset, input_curr_node_data->key, curr_physic_node.offsets.physic_next);
#endif
				new_physic_node_offset = physic_offset;
			}

			// 1 0, 正在使用的最后一个节点
			else if (1 == curr_physic_node.used && first_physic_node_offset == curr_physic_node.offsets.physic_next) {
				// 新节点在文件末尾插入，获取新节点偏移量
				if ((new_physic_node_offset = lseek(fd, 0, SEEK_END)) < 0) {
					hash_error("prepare new curr_physic_node, seek to %ld fail : %s.",
							physic_offset, strerror(errno));
					goto close_file;
				}

				/**** 1. START 修改 当前 节点的next_offset值，指向新节点 ****/
				lseek(fd, physic_offset, SEEK_SET);

				curr_physic_node.offsets.physic_next = new_physic_node_offset;

				if (write(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
					hash_error("write curr_physic_node error : %s.", strerror(errno));
					goto close_file;
				}
				/**** 1. END 修改 当前 节点的next_offset值，指向新节点 ****/

				/**** 2. START 修改 头 节点的prev_offset值，指向新节点 ****/
				lseek(fd, first_physic_node_offset, SEEK_SET);


				if (read(fd, &first_physic_node, sizeof(hash_node_t)) < 0) {
					hash_error("read curr_physic_node error : %s.", strerror(errno));
					goto close_file;
				}

				first_physic_node.offsets.physic_prev = new_physic_node_offset;

				// 再次回到节点开头写回
				lseek(fd, first_physic_node_offset, SEEK_SET);

				if (write(fd, &first_physic_node, sizeof(hash_node_t)) < 0) {
					hash_error("write curr_physic_node error : %s.", strerror(errno));
					goto close_file;
				}
				/**** 2. END 修改 头 节点的prev_offset值，指向新节点 ****/

				/**** 3. START 修改 新 节点的prev和next指针 ****/
				lseek(fd, 0, SEEK_END);		// 新节点在尾部

				curr_physic_node.offsets.physic_prev = physic_offset;
				curr_physic_node.offsets.physic_next = first_physic_node_offset;
				/**** 3. END 修改 新 节点的prev和next指针 ****/

#if MORE_ADD_NODE_INFO
				hash_debug(" (TAIL) <0x%lX> (0x%lX : %d) <0x%lX>",
						curr_physic_node.offsets.physic_prev, new_physic_node_offset, input_curr_node_data->key, curr_physic_node.offsets.physic_next);
#endif
			}

			/**** 4. START 写入新节点的其他信息 ****/
			++header.slots[which_slot].node_cnt;

			curr_physic_node.used = 1;

			addr = curr_physic_node.data.value;
			memcpy(&(curr_physic_node.data), input_curr_node_data, sizeof(hash_node_data_t));

			curr_physic_node.data.value = addr;
			memcpy(curr_physic_node.data.value, input_curr_node_data->value, node_data_value_size);

			/* START 调整逻辑链表。上面已完成调整物理链表 */
			// 第一个节点。
			if (true == is_first_node) {
				header.slots[which_slot].first_logic_node_offset = physic_offset;
				curr_physic_node.offsets.logic_prev = curr_physic_node.offsets.logic_next = new_physic_node_offset;
#if MORE_ADD_NODE_INFO
				hash_debug("first node offset 0x%lX.", new_physic_node_offset);
#endif
			} else {
				/*
				 * 双向链表插入，curr为待插入节点
				 * nextNode->prev = curr;
				 * prevNode->next = curr;
				 * currNode->next = nextNode;
				 * currNode->prev = prevNode;
				 */

				/* START 4.1. 读取 next prev 节点操作 */
#if MORE_ADD_NODE_INFO
				hash_debug("new node 0x%lX inset behind 0x%lX, before add 0x%lX <- 0x%lX -> 0x%lX.",
						new_physic_node_offset, prev_logic_node_offset,
						prev_logic_node.offsets.logic_prev, prev_logic_node_offset, prev_logic_node.offsets.logic_next);
#endif

				// prev 节点
				if (lseek(fd, prev_logic_node_offset, SEEK_SET) < 0) {
					hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
					goto close_file;
				}

				if (read(fd, &prev_logic_node, sizeof(hash_node_t)) < 0) {
					hash_error("read next_logic_node error : %s.", strerror(errno));
					goto close_file;
				}

				// next 节点。如果prev和next相等，说明当前只有一个节点，后面会有多个这种判断
				if (prev_logic_node_offset != (next_logic_node_offset = prev_logic_node.offsets.logic_next)) {
					if (lseek(fd, next_logic_node_offset, SEEK_SET) < 0) {
						hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
						goto close_file;
					}

					if (read(fd, &next_logic_node, sizeof(hash_node_t)) < 0) {
						hash_error("read next_logic_node error : %s.", strerror(errno));
						goto close_file;
					}
				}
				/* END 4.1. 读取 next prev 节点操作 */

				/* START 4.2. 重新建立节点链接 */
				prev_logic_node.offsets.logic_next = new_physic_node_offset;

				if (prev_logic_node_offset != next_logic_node_offset) {
					next_logic_node.offsets.logic_prev = new_physic_node_offset;
				} else {		// 在只有一个节点的情况下插入
					prev_logic_node.offsets.logic_next = new_physic_node_offset;
				}

				curr_physic_node.offsets.logic_next = next_logic_node_offset;
				curr_physic_node.offsets.logic_prev = prev_logic_node_offset;
				/* END 4.2. 重新建立节点链接 */

#if MORE_ADD_NODE_INFO
				hash_debug("prevNode : 0x%lX <- (0x%lX) -> 0x%lX",
						prev_logic_node.offsets.logic_prev, prev_logic_node_offset, prev_logic_node.offsets.logic_next);

				hash_debug("currNode : 0x%lX <- (0x%lX) -> 0x%lX",
						curr_physic_node.offsets.logic_prev, new_physic_node_offset, curr_physic_node.offsets.logic_next);

				// 在只有一个节点的情况下插入
				if (prev_logic_node_offset != next_logic_node_offset) {
					hash_debug("nextNode : 0x%lX <- (0x%lX) -> 0x%lX",
							next_logic_node.offsets.logic_prev, next_logic_node_offset, next_logic_node.offsets.logic_next);
				}
#endif
				/* START 4.3. 写回到文件 */
				if (lseek(fd, prev_logic_node_offset, SEEK_SET) < 0) {
					hash_error("seek to %ld fail : %s.", physic_offset, strerror(errno));
					goto close_file;
				}

				if (write(fd, &prev_logic_node, sizeof(hash_node_t)) < 0) {
					hash_error("write next_logic_node error : %s.", strerror(errno));
					goto close_file;
				}

				if (prev_logic_node_offset != next_logic_node_offset) {
					if (lseek(fd, next_logic_node_offset, SEEK_SET) < 0) {
						hash_error("seek to %ld fail : %s.", next_logic_node_offset, strerror(errno));
						goto close_file;
					}

					if (write(fd, &next_logic_node, sizeof(hash_node_t)) < 0) {
						hash_error("write next_logic_node error : %s.", strerror(errno));
						goto close_file;
					}
				}
				/* END 4.3. 写回到文件 */
			}

			/* END 完成调整逻辑链表 */

			if (lseek(fd, new_physic_node_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", new_physic_node_offset, strerror(errno));
				goto close_file;
			}

			if (write(fd, &curr_physic_node, sizeof(hash_node_t)) < 0) {
				hash_error("write curr_physic_node error : %s.", strerror(errno));
				goto close_file;
			}

			if (node_data_value_size > 0
					&& write(fd, curr_physic_node.data.value, node_data_value_size) < 0) {
				hash_error("write node_data_value error : %s.", strerror(errno));
				goto close_file;
			}
			/**** 4. END 写入新节点的其他信息 ****/
			break;
		} else {
			physic_offset = curr_physic_node.offsets.physic_next;
		}
	}  while (physic_offset != first_physic_node_offset);

	/* START 保存头部信息 */
	if (lseek(fd, sizeof(hash_header_t), SEEK_SET) < 0) {
		hash_error("seek to %ld fail : %s.", sizeof(hash_header_t), strerror(errno));
		goto close_file;
	}

	if (write(fd, header.slots, slot_cnt * sizeof(slot_info_t)) < 0) {
		hash_error("write header.slots error : %s.", strerror(errno));
		goto close_file;
	}
	/* END 保存头部信息 */

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(header.slots);
	safe_free(node_data_value);
	return ret;
}
#undef MORE_ADD_NODE_INFO

#define MORE_DEL_NODE_INFO 0
int del_node(const char* path, hash_node_data_t* input_node_data,
		int (*cb)(hash_node_data_t*, hash_node_data_t*)) {
	int ret = -1;
	int fd = 0;
	uint32_t which_slot = 0;
	off_t offset = 0;
	off_t first_logic_node_offset = 0;
	off_t prev_logic_node_offset = 0;
	off_t next_logic_node_offset = 0;
	hash_header_t header;
	slot_info_t* slots = NULL;
	hash_node_t node;
	hash_node_t prev_logic_node;
	hash_node_t next_logic_node;
	void* node_data_value = NULL;
	uint32_t slot_cnt = 0;
	uint32_t node_data_value_size = 0;
	void *addr = NULL;	// 防止在memcpy中，文件中保存的上一次指针值覆盖了当前正在运行的指针

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

	slot_cnt = header.slot_cnt;
	node_data_value_size = header.node_data_value_size;

	if (NULL == (slots = (void*)calloc(slot_cnt, sizeof(slot_info_t)))) {
		hash_error("calloc failed.");
		goto exit;
	}

	if (read(fd, slots, slot_cnt * sizeof(slot_info_t)) < 0) {
		hash_error("read slot_info error : %s.", strerror(errno));
		goto close_file;
	}

	header.slots = slots;

	which_slot = input_node_data->key % slot_cnt;
	first_logic_node_offset = header.slots[which_slot].first_logic_node_offset;

	if (node_data_value_size > 0
			&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	offset = first_logic_node_offset;
	do {
		if (lseek(fd, offset, SEEK_SET) < 0) {
			hash_error("seek to %ld fail : %s.", offset, strerror(errno));
			goto close_file;
		}

		if (read(fd, &node, sizeof(hash_node_t)) < 0) {
			hash_error("read node failed : %s.", strerror(errno));
			goto close_file;
		}

		if (node_data_value_size > 0
				&& read(fd, node_data_value, node_data_value_size) < 0) {
			hash_error("read node_data_value failed : %s.", strerror(errno));
			goto close_file;
		}

		// 建立关联，方便后面使用。之后不要破坏这种关联（比如read调用）
		node.data.value = node_data_value;

		// 找到了节点
		if (0 == cb(&(node.data), input_node_data)) {
			node.used = 0;
			--header.slots[which_slot].node_cnt;

			/*
			 * 双向链表删除，curr为待插入节点
			 * nextNode->prev = prevNode;
			 * prevNode->next = nextNode;
			 */
			/* START 调整逻辑链表 */
			/* START 1. 读取 prev next 节点信息*/
			// prev 节点
			prev_logic_node_offset = node.offsets.logic_prev;
			if (lseek(fd, prev_logic_node_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", prev_logic_node_offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &prev_logic_node, sizeof(hash_node_t)) < 0) {
				hash_error("read next_logic_node error : %s.", strerror(errno));
				goto close_file;
			}

			// next 节点。如果prev和next相等，说明当前只有一个节点，后面会有多个这种判断
			next_logic_node_offset = node.offsets.logic_next;
			if (lseek(fd, next_logic_node_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", next_logic_node_offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &next_logic_node, sizeof(hash_node_t)) < 0) {
				hash_error("read next_logic_node error : %s.", strerror(errno));
				goto close_file;
			}
			/* END 1. 读取 prev next 节点信息*/
			/* START 2. 修改节点链式关系 */

			// 仅剩 一个 节点
			if (offset == prev_logic_node_offset && offset == next_logic_node_offset) {
				hash_debug("only 1 node 0x%lX left.", offset);
				header.slots[which_slot].first_logic_node_offset = node.offsets.logic_next;
				goto clear_node;
			} else {
				if (offset == first_logic_node_offset) {	// 删除逻辑第一个节点
#if MORE_DEL_NODE_INFO
					hash_debug("delete first logic node 0x%lX, update first logic node to 0x%lX.",
							offset, node.offsets.logic_next);
#endif
					header.slots[which_slot].first_logic_node_offset = node.offsets.logic_next;
				} else {
#if MORE_DEL_NODE_INFO
					hash_debug("delete normal node 0x%lX.", offset);
#endif
				}

				// 剩两个节点
				if (prev_logic_node_offset == next_logic_node_offset) {
#if MORE_DEL_NODE_INFO
					hash_debug("2 nodes left.");
#endif
					prev_logic_node.offsets.logic_prev = prev_logic_node_offset;
					prev_logic_node.offsets.logic_next = prev_logic_node_offset;
				}
				
				// 更多节点
				else {
					next_logic_node.offsets.logic_prev = prev_logic_node_offset;
					prev_logic_node.offsets.logic_next = next_logic_node_offset;
				}
			}

			/* END 2. 修改节点链式关系 */
#if MORE_DEL_NODE_INFO
			hash_info("after del 0x%lX, prev = ( 0x%lX <- 0x%lX -> 0x%lX ), next = ( 0x%lX <- 0x%lX -> 0x%lX ).", offset,
					prev_logic_node.offsets.logic_prev, prev_logic_node_offset, prev_logic_node.offsets.logic_next,
					next_logic_node.offsets.logic_prev, next_logic_node_offset, next_logic_node.offsets.logic_next);
#endif
			/* START 3. 写回到文件 */
			if (lseek(fd, prev_logic_node_offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", prev_logic_node_offset, strerror(errno));
				goto close_file;
			}

			if (write(fd, &prev_logic_node, sizeof(hash_node_t)) < 0) {
				hash_error("write next_logic_node error : %s.", strerror(errno));
				goto close_file;
			}

			if (prev_logic_node_offset != next_logic_node_offset) {
				if (lseek(fd, next_logic_node_offset, SEEK_SET) < 0) {
					hash_error("seek to %ld fail : %s.", next_logic_node_offset, strerror(errno));
					goto close_file;
				}

				if (write(fd, &next_logic_node, sizeof(hash_node_t)) < 0) {
					hash_error("write next_logic_node error : %s.", strerror(errno));
					goto close_file;
				}
			}
			/* END 3. 写回到文件 */
			/* END 完成调整逻辑链表 */

clear_node:
			/* START 清空当前节点 */
			// 移到节点起始位置
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek back to %ld fail : %s.", offset, strerror(errno));
				goto close_file;
			}

			addr = node.data.value;
			node.used = 0;
			memset(&(node.data), 0, sizeof(hash_node_data_t));
			if (write(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("del node error : %s.", strerror(errno));
				goto close_file;
			}

			node.data.value = addr;
			memset(node.data.value, 0, node_data_value_size);
			if (node_data_value_size > 0
					&& write(fd, node.data.value, node_data_value_size) < 0) {
				hash_error("del node.data.value error : %s.", strerror(errno));
				goto close_file;
			}
			/* END 清空当前节点 */

			/* START 保存头部信息 */
			if (lseek(fd, sizeof(hash_header_t), SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", sizeof(hash_header_t), strerror(errno));
				goto close_file;
			}

			if (write(fd, header.slots, slot_cnt * sizeof(slot_info_t)) < 0) {
				hash_error("write header.slots error : %s.", strerror(errno));
				goto close_file;
			}
			/* END 保存头部信息 */

			ret = 0;

			break;
		}

		offset = node.offsets.logic_next;
	} while (offset != first_logic_node_offset);

close_file:
	close(fd);

exit:
	safe_free(slots);
	safe_free(node_data_value);
	return ret;
}
#undef MORE_DEL_NODE_INFO

// traverse_type 为 TRAVERSE_ALL 时，hash_key可随意填写
uint8_t traverse_nodes(const char* path, traverse_type_t traverse_type, traverse_by_what_t by_what,
		uint32_t hash_key, printable_t print, hash_node_data_t* input_node_data,
		traverse_action_t (*cb)(hash_node_t*, hash_node_data_t*)) {
	traverse_action_t action = TRAVERSE_ACTION_DO_NOTHING;
	uint8_t i = 0;
	int fd = 0;
	off_t offset = 0;
	off_t first_node_offset = 0;
	off_t first_physic_node_offset = 0;
	off_t first_logic_node_offset = 0;
	hash_header_t header;
	slot_info_t* slots = NULL;
	hash_node_t node;
	void* node_data_value = NULL;
	uint32_t slot_cnt = 0;
	uint32_t header_data_value_size = 0;
	uint32_t node_data_value_size = 0;
	uint8_t break_or_not = 0;
	static uint8_t s_first_node = 1;

	memset(&header, 0, sizeof(hash_header_t));
	memset(&node, 0, sizeof(hash_node_t));

	if ((fd = open(path, O_RDWR)) < 0) {
		hash_error("open %s fail : %s.", path, strerror(errno));
		goto exit;
	}

	// 先读取头部的哈希信息
	if (read(fd, &header, sizeof(hash_header_t)) < 0) {
		hash_error("read header error : %s.", strerror(errno));
		goto close_file;
	}

	slot_cnt = header.slot_cnt;
	header_data_value_size = header.header_data_value_size;
	node_data_value_size = header.node_data_value_size;

	if (NULL == (slots = (void*)calloc(slot_cnt, sizeof(slot_info_t)))) {
		hash_error("calloc failed.");
		goto exit;
	}

	if (read(fd, slots, slot_cnt * sizeof(slot_info_t)) < 0) {
		hash_error("read slot_info error : %s.", strerror(errno));
		goto close_file;
	}

	header.slots = slots;

	if (node_data_value_size > 0
			&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
		hash_error("calloc failed.");
		goto exit;
	}

	for (i = 0; i < slot_cnt; i++) {
		/*if (0 == header.slots[i].node_cnt) {
			hash_warn("no node in slot %d.", i);
			continue;
		}*/

		s_first_node = 1;

		// TODO: hash_key和i的关系不一定可以直接比较，后续版本需要完善
		if (TRAVERSE_SPECIFIC_HASH_SLOT == traverse_type && i != (hash_key % slot_cnt)) {
			continue;
		}

		first_physic_node_offset = sizeof(hash_header_t) + slot_cnt * sizeof(slot_info_t) + header_data_value_size\
			 + i * (sizeof(hash_node_t) + node_data_value_size);
		first_logic_node_offset = header.slots[i].first_logic_node_offset;

		first_node_offset = TRAVERSE_BY_LOGIC == by_what ? first_logic_node_offset : first_physic_node_offset;

		if (WITH_PRINT == print) { printf("[%d] %s  ", i, TRAVERSE_BY_LOGIC == by_what ? " \e[7;32mLOGIC\e[0m" : "\e[7;34mPHYSIC\e[0m"); }

		offset = first_node_offset;
		do {
			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", offset, strerror(errno));
				goto close_file;
			}

			if (read(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("read node failed : %s.", strerror(errno));
				goto close_file;
			}

			if (node_data_value_size > 0
					&& read(fd, node_data_value, node_data_value_size) < 0) {
				hash_error("read node_data_value failed : %s.", strerror(errno));
				goto close_file;
			}

			node.data.value = node_data_value;

			if (s_first_node) {
				s_first_node = 0;
			} else {
				if (WITH_PRINT == print) { printf(" --- "); }
			}

			if (WITH_PRINT == print) { printf("<0x%lX> ( 0x%lX : ", TRAVERSE_BY_LOGIC == by_what ? node.offsets.logic_prev : node.offsets.physic_prev, offset); }

			action = cb(&node, input_node_data);

			if (WITH_PRINT == print) { printf(" ) <0x%lX>", TRAVERSE_BY_LOGIC == by_what ? node.offsets.logic_next : node.offsets.physic_next); }

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

				if (node_data_value_size > 0
						&& write(fd, node.data.value, node_data_value_size) < 0) {
					hash_error("del node_data_value error : %s.", strerror(errno));
					goto close_file;
				}
			}

			if (TRAVERSE_ACTION_BREAK & action) {
				break_or_not = 1;
				goto close_file;
			}

			offset = TRAVERSE_BY_LOGIC == by_what ? node.offsets.logic_next : node.offsets.physic_next;
		} while (offset != first_node_offset);

		if (WITH_PRINT == print) { printf("\n"); }
	}

close_file:
	close(fd);

exit:
	safe_free(slots);
	safe_free(node_data_value);
	return break_or_not;
}

int init_hash_engine(const char* path, init_method_t rebuild,
		int slot_cnt, int node_data_value_size, int header_data_value_size) {
	int ret = -1;
	int fd = 0;
	uint32_t i = 0;
	uint8_t file_exist = 0;
	hash_header_t header;
	slot_info_t* slots = NULL;
	void* header_data_value = NULL;
	hash_node_t node;
	void* node_data_value = NULL;
	off_t offset = 0;

	hash_info("path = %s, rebuild = %d, "
			"slot_cnt = %d, node_data_value_size = %d, header_data_value_size = %d.",
			path, rebuild, slot_cnt, node_data_value_size, header_data_value_size);

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
		if (NULL == (slots = (void*)calloc(slot_cnt, sizeof(slot_info_t)))) {
			hash_error("calloc failed.");
			goto exit;
		}

		if (header_data_value_size > 0
				&& NULL == (header_data_value = (void*)calloc(1, header_data_value_size))) {
			hash_error("calloc failed.");
			goto exit;
		}

		header.slot_cnt = slot_cnt;
		header.header_data_value_size = header_data_value_size;
		header.node_data_value_size = node_data_value_size;
		header.slots = slots;
		header.data.value = header_data_value;

		if (node_data_value_size > 0
				&& NULL == (node_data_value = (void*)calloc(1, node_data_value_size))) {
			hash_error("calloc failed.");
			goto close_file;
		}

		node.data.value = node_data_value;

		for (i = 0; i < slot_cnt; i++) {
			offset = sizeof(hash_header_t) + slot_cnt * sizeof(slot_info_t) + header_data_value_size\
				 + i * (sizeof(hash_node_t) + node_data_value_size);
			node.offsets.physic_prev = node.offsets.physic_next = offset;
			node.offsets.logic_prev = node.offsets.logic_next = offset;

			header.slots[i].first_logic_node_offset = offset;

			if (lseek(fd, offset, SEEK_SET) < 0) {
				hash_error("seek to %ld fail : %s.", offset, strerror(errno));
				goto close_file;
			}

			if (write(fd, &node, sizeof(hash_node_t)) < 0) {
				hash_error("init node error : %s.", strerror(errno));
				goto close_file;
			}

			if (node_data_value_size > 0
					&& write(fd, node.data.value, node_data_value_size) < 0) {
				hash_error("init node.data.value error : %s.", strerror(errno));
				goto close_file;
			}
		}

		if (lseek(fd, 0, SEEK_SET) < 0) {
			hash_error("seek to head fail : %s.", strerror(errno));
			goto close_file;
		}

		if (write(fd, &header, sizeof(hash_header_t)) < 0) {
			hash_error("write header error : %s.", strerror(errno));
			goto close_file;
		}

		if (write(fd, header.slots, slot_cnt * sizeof(slot_info_t)) < 0) {
			hash_error("write header.slots error : %s.", strerror(errno));
			goto close_file;
		}

		if (header_data_value_size > 0
				&& write(fd, header.data.value, header_data_value_size) < 0) {
			hash_error("write header.data.value error : %s.", strerror(errno));
			goto close_file;
		}
	}

	ret = 0;

close_file:
	close(fd);

exit:
	safe_free(slots);
	safe_free(header_data_value);
	safe_free(node_data_value);
	return ret;
}
