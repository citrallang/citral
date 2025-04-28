#pragma once
#include "allocator.h"
#include "stdint.h"
//ALL METHODS HERE SHOULD BE STATIC
//traditional quadratically probed hash table.

typedef union hash_keyval {
	void* asPtr;
	uint64_t asui64;
	double asdouble;
	int64_t asi64;
} hash_keyval;

typedef struct hash_node {
	hash_keyval key;
	hash_keyval val;
	unsigned int keysize;
	unsigned int valsize;
} hash_node;


