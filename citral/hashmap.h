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
	int keysize;
	int valsize;
} hash_node;

typedef struct hash_node_sizeless {
	hash_keyval key;
	hash_keyval val;
};

//SAFE: evaluates a once
#define hash_node_as_sizeless(a) (*((hash_node_sizeless*)(&(a))))
#define hash_node_as_sized(a) (*((hash_node*)(&(a))))
