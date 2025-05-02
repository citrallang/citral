#pragma once
#include "stdlib.h"
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

typedef enum HashValType {
	TYPE_POINTER, TYPE_UINT64, TYPE_DOUBLE, TYPE_I64,
} HashValType;

typedef struct HashNode {
	HashKeyVal key;
	long hash;
	HashKeyVal val;
	unsigned int keySize; //these can be used for additional info if the type bits are set to the non pointer values
	unsigned int valSize;
	unsigned int keyType : 2;
	unsigned int valType : 2;
	unsigned int isGrave : 1;
	unsigned int isFull : 1;
} HashNode;

typedef struct HashTable {
	HashNode* nodes;
	unsigned int numNodes;
	unsigned int maxNodes;
	unsigned int usePrimitiveHasher : 1;
} HashTable;

static long hash_str(char*, unsigned int);
static HashTable* spawn_hashtable();
static void free_hashtable(HashTable*);
static unsigned int internal_insert_into_hashtable(HashTable*, HashKeyVal, HashKeyVal, unsigned int, unsigned int, HashValType, HashValType);
static HashKeyVal internal_remove_from_hashtable(HashTable*, HashKeyVal, unsigned int);
static void resize_hashtable(HashTable*, unsigned int);
static unsigned int internal_get_first_empty(HashTable* tbl, long hash);
static unsigned int internal_get_pos_of_element(HashTable*, HashKeyVal, unsigned int);
static unsigned int internal_get_pos_of_element_with_hash(HashTable*, HashKeyVal, uint32_t, long);

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

static void free_hashtable(HashTable* tbl) {
	free(tbl->nodes);
	free(tbl);
}

static HashTable* spawn_hashtable() {
	HashTable* tbl = xmalloc(sizeof(HashTable));
	tbl->maxNodes = 16;
	tbl->nodes = xmalloc(sizeof(HashNode) * 16);
	return tbl;
}

static long get_hash(HashTable* tbl, HashKeyVal key, unsigned int keySize) {
	if (!tbl->usePrimitiveHasher) {
		return hash_str(key.asPtr, keySize);
	}
	return *(long*)0;
}

static unsigned int internal_get_pos_of_element(HashTable* tbl, HashKeyVal key, unsigned int keySize) {
	if (!tbl->usePrimitiveHasher)
		return internal_get_pos_of_element_with_hash(tbl, key, keySize, hash_str(key.asPtr, keySize));
	//else
		//return internal_get_pos_of_element_with_hash(tbl, key, keySize, hash_str(key.asPtr, keySize));
}

static unsigned int internal_get_pos_of_element_with_hash(HashTable* tbl, HashKeyVal key, uint32_t keySize, long hash) {
	long step = 0;
	long currentIndex = hash % tbl->maxNodes;
	for (;;) {
		HashNode currentNode = tbl->nodes[currentIndex];
		if (!currentNode.isFull && !currentNode.isGrave) {
			return UINT32_MAX;
		}
		if (currentNode.hash == hash) {
			if (key.asI64 == currentNode.key.asI64) {
				return currentIndex;
			}
			if (!tbl->usePrimitiveHasher) {
				if (keySize == currentNode.keySize) {
					if (memcmp(key.asPtr, currentNode.key.asPtr, keySize) == 0) {
						return currentIndex;
					}
				}
			}
		}
		step++;
		currentIndex = (currentIndex + (step ^ 2)) % tbl->maxNodes;
	}
}

static unsigned int internal_get_first_empty(HashTable* tbl, long hash) {
	long step = 0;
	long currentIndex = hash % tbl->maxNodes;
	for (;;) {
		if (!tbl->nodes[currentIndex].isFull) {
			return currentIndex;
		}
		step++;
		currentIndex = (currentIndex + (step ^ 2)) % tbl->maxNodes;
	}
}

static unsigned int internal_insert_into_hashtable(HashTable* tbl, HashKeyVal key, HashKeyVal val, unsigned int keySize, unsigned int valSize, HashValType keyType, HashValType valType) {
	long hash = get_hash(tbl, key, keySize);
	unsigned int firstSlot = internal_get_first_empty(tbl, hash);
	HashNode t = {
		.hash = hash,
		.key = key,
		.keySize = keySize,
		.keyType = keyType,
		.val = val,
		.valSize = valSize,
		.valType = valType,
	};
	tbl->nodes[firstSlot] = t;
	tbl->numNodes++;
	if (++(tbl->numNodes) > tbl->maxNodes * 0.6) {
		resize_hashtable(tbl, tbl->maxNodes * 1.5);
	}
	return firstSlot;
}

static void resize_hashtable(HashTable* tbl, unsigned int newSize) {
	size_t new_amt = newSize * sizeof(HashNode);
	HashNode* new_nodes = xmalloc(new_amt);
	HashTable newtbl = {
		.nodes = new_nodes,
		.maxNodes = newSize,
		.numNodes = 0,
		.usePrimitiveHasher = tbl->usePrimitiveHasher
	};
	HashTable* tbl_p = &newtbl;
	for (int i = 0; i < tbl->maxNodes; i++) {
		HashNode cur = tbl->nodes[i];
		if (cur.isFull) {
			internal_insert_into_hashtable(tbl_p, cur.key, cur.val, cur.keySize, cur.valSize, cur.keyType, cur.valType);
		}
	}
	tbl->nodes = new_nodes;
	tbl->maxNodes = new_amt;
}

static HashKeyVal internal_remove_from_hashtable(HashTable* tbl, HashKeyVal key, unsigned int keySize) {
	unsigned int pos = internal_get_pos_of_element(tbl, key, keySize);
	tbl->nodes[pos].isGrave = 1;
	tbl->numNodes--;
	return tbl->nodes[pos].val;
}