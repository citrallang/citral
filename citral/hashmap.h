#pragma once
#include "allocator.h"
#include "stdint.h"
//ALL METHODS HERE SHOULD BE STATIC
//traditional quadratically probed hash table.

typedef union HashKeyVal {
	void* asPtr;
	uint64_t asUI64;
	double asDouble;
	int64_t asI64;
} HashKeyVal;

typedef struct HashNode {
	HashKeyVal key;
	HashKeyVal val;
	unsigned int keySize; //these can be used for additional info if the type bits are set to the non pointer values
	unsigned int valsize;
	unsigned int keyType : 2;
	unsigned int valType : 2;
	unsigned int isGrave : 1;
	unsigned int isEmpty : 1;
} HashNode;

typedef union HashFunc {
	long (*asPtrFunc)(char*, unsigned int);
	long (*asLongFunc)(long);
} HashFunc;

typedef struct HashTable {
	HashNode* nodes;
	unsigned int numNodes;
	unsigned int maxNodes;
	HashFunc hasher; //idk if i want to do this but hey
} HashTable;

//bad hash function, will replace later
//todo
long hash_str(char* str, unsigned int len) {
	long hash = 0;
	for (int i = 0; i < len; i++) {
		hash <<= 7;
		hash += (str[i]*13);
	}
	return hash;
}

static HashTable* spawn_hashtable() {
	HashTable* tbl = xmalloc(sizeof(HashTable));
	//tbl->hasher = INSERT_HASHER_HERE
	//todo
	tbl->maxNodes = 16;
	tbl->nodes = xmalloc(sizeof(HashNode) * 16);
	return tbl;
}