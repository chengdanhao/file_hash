#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>

int append_book(char* record_path, uint32_t bookcode, char* path);
void print_nodes(char* path);

#endif
