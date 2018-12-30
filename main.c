#include <stdio.h>
#include "book_hash.h"

typedef struct {
	char* path;
	int code;
} book_t;

#define BOOK_RECORD "record"

int main() {
	const book_t books[] = {
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

	printf("%lu books in total.\n", sizeof(books)/ sizeof(book_t));
	for (int i = 0; i < sizeof(books) / sizeof(book_t); i++) {
		append_book(BOOK_RECORD, books[i].code, books[i].path);
	}

	print_nodes(BOOK_RECORD);
}
