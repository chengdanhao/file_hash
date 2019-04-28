#ifndef __BOOKCODE_HASH__
#define __BOOKCODE_HASH__

#include <stdint.h>
#include "node.h"

int add_node(char* record_path, node_data_t* data, int (*cb)(node_data_t*, node_data_t*));
int del_node(char* record_path, node_data_t* data, int (*cb)(node_data_t*, node_data_t*));
void print_nodes(char* path, char* (*cb)(node_data_t*));

#endif
