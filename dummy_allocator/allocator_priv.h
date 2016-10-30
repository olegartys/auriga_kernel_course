#ifndef ALLOCATOR_PRIV_H
#define ALLOCATOR_PRIV_H

#include <stdint.h>

#define DEBUG 0

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

/**
 * Преобразование адреса из адреса chunk в адрес памяти, которым он 
 * управляет, и наоборот
 */
#define CHUNK_MEM(chunk) (void*)((char*)chunk + CHUNK_SIZE)
#define MEM_CHUNK(ptr) (mem_chunk_t*)((char*)ptr - CHUNK_SIZE)

#endif // ALLOCATOR_PRIV_H
