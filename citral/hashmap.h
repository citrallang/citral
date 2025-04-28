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
	unsigned int keysize; //these can be used for additional info if the type bits are set to the non pointer values
	unsigned int valsize;
	unsigned int key_type : 2;
	unsigned int val_type : 2;
	unsigned int isGrave : 1;
	unsigned int isEmpty : 1;
} hash_node;


