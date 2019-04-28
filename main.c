#include <stdio.h>
#include <string.h>
#include "hash.h"

typedef struct {
	char* path;
	int code;
} book_t;

#define BOOK_RECORD "record"

int main() {
	const book_t books_1[] = {
		{"0", 0},
		{"1", 1},
		{"222", 8},
		{"11", 4},
		{"22", 5},
		{"3", 3},
		{"111", 7},
		{"2", 2},
		{"1111", 10},
		{"2222", 11},
		{"33", 6},
		{"11111", 13},
		{"333", 9},
	};

	const book_t books_2[] = {
		{"new 35", 35},
		{"new 33", 33},
		{"new 34", 34},
		{"new 30", 30},
		{"new 13", 13},
		{"new 11", 11},
		{"new 9", 9},
	};

	printf("%lu books in total.\n", sizeof(books_1)/ sizeof(book_t));
	for (int i = 0; i < sizeof(books_1) / sizeof(book_t); i++) {
		node_data_t data;
		data.hash_key = data.book_code = books_1[i].code;
		strncpy(data.path, books_1[i].path, sizeof(data.path));
		add_node(BOOK_RECORD, &data, add_node_cb);
	}

	print_nodes(BOOK_RECORD);

	node_data_t data;
	data.hash_key = data.book_code = 0;
	del_node(BOOK_RECORD, &data, del_node_cb);
	data.hash_key = data.book_code = 4;
	del_node(BOOK_RECORD, &data, del_node_cb);
	data.hash_key = data.book_code = 11;
	del_node(BOOK_RECORD, &data, del_node_cb);
	data.hash_key = data.book_code = 3;
	del_node(BOOK_RECORD, &data, del_node_cb);

	print_nodes(BOOK_RECORD);

	for (int i = 0; i < sizeof(books_2) / sizeof(book_t); i++) {
		node_data_t data;
		data.hash_key = data.book_code = books_2[i].code;
		strncpy(data.path, books_2[i].path, sizeof(data.path));
		add_node(BOOK_RECORD, &data, add_node_cb);
	}

	print_nodes(BOOK_RECORD);
}
