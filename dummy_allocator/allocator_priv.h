#ifndef ALLOCATOR_PRIV_H
#define ALLOCATOR_PRIV_H

#include <stdint.h>

/**
 * Структура, описывающая метаинформацию о блоке памяти
 */
struct _mem_chunk_t {
	size_t size;
	struct _mem_chunk_t *next;
	int is_free;
};

typedef struct _mem_chunk_t mem_chunk_t;

#define CHUNK_SIZE (sizeof (mem_chunk_t))

#endif // ALLOCATOR_PRIV_H
