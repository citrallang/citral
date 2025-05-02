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
	long hash;
	HashKeyVal val;
	unsigned int keySize; //these can be used for additional info if the type bits are set to the non pointer values
	unsigned int valSize;
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
	unsigned int hashFuncUsed : 1;
} HashTable;

static long hash_str(char* str, unsigned int len);
static HashTable* spawn_hashtable();
static void free_hashtable();
static uint8_t internal_insert_into_hashtable(HashTable* tbl, HashKeyVal key, HashKeyVal value, unsigned int keySize, unsigned int valSize);
static void internal_remove_from_hashtable(HashTable* tbl, HashKeyVal key, unsigned int keySize);
static void resize_hashtable(HashTable* tbl, unsigned int newSize);
static unsigned int internal_get_pos_of_element(HashTable* tbl, HashKeyVal key, unsigned int keySize);
static unsigned int internal_get_pos_of_element_with_hash(HashTable* tbl, HashKeyVal key, unsigned int keySize, long hash);

//bad hash function, will replace later
//todo
static long hash_str(char* str, unsigned int len) {
	long hash = 0;
	for (int i = 0; i < len; i++) {
		hash <<= 7;
		hash += (str[i]*13);
	}
	return hash;
}

static HashTable* spawn_hashtable() {
	HashTable* tbl = xmalloc(sizeof(HashTable));
	tbl->hasher.asLongFunc = hash_str;
	tbl->maxNodes = 16;
	tbl->nodes = xmalloc(sizeof(HashNode) * 16);
	return tbl;
}

static unsigned int internal_get_pos_of_element(HashTable* tbl, HashKeyVal key, unsigned int keySize, uint8_t func) {
	long hash;
	if (func == 0)
		hash = tbl->hasher.asPtrFunc(key.asPtr, keySize);
	else
		hash = tbl->hasher.asLongFunc(key.asI64);
	return internal_get_pos_of_element_with_hash(tbl, key, keySize, hash);
}

static unsigned int internal_get_pos_of_element_with_hash(HashTable* tbl, HashKeyVal key, unsigned int keySize, long hash) {
	long step = 0;
	long current = hash % tbl->maxNodes;
	for (;;) {
		

		step++;
		current = (current + (step ^ 2)) % tbl->maxNodes;
	}
}