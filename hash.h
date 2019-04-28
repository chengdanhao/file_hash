#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>

int add_book(char* record_path, uint32_t bookcode, char* path);
int del_book(char* record_path, uint32_t book_code);
void print_nodes(char* path);

#endif
