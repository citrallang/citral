#pragma once
#include <stdlib.h>
#include <string.h> //why is memset here
static inline void* xmalloc_noinit(size_t size);

static inline void* xmalloc(size_t size) {
	void* block = xmalloc_noinit(size);
	memset(block, 0, size);
	return block;
}

static inline void* xmalloc_noinit(size_t size) {
	void* block = malloc(size);
	if (block == NULL) {
		perror("Memory allocation failed.");
		exit(1);
	}
	return block;
}

static inline void* xrealloc(void* block, size_t size) {
	void* t = realloc(block, size);
	if (t == NULL) {
		perror("Realloc failed.");
		exit(1);
	}
	return t;
}
#define xxrealloc(a, b) _xxrealloc((void**)a, b)
static inline void _xxrealloc(void** block, size_t size) {
	void* t = xrealloc(*block, size);
	*block = t;
}