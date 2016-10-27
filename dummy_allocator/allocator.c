#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "allocator_priv.h"

static mem_chunk_t *base = NULL;
static mem_chunk_t **last = NULL;

/**
 *  Выравнивание по границе 8 байт
 */
//FIXME: платформозависимо.
inline static size_t _align(size_t size) {
	return size - size % 8 + 8;
}

/**
 * Поиск пододящего блока памяти
 */
static mem_chunk_t *find_chunk(size_t size) {
	mem_chunk_t *_base = base;
	
	while (_base && !(_base->is_free && _base->size >= size)) {
		_base = _base->next;
	}
	
	if (_base) {
		_base->is_free = 0; // FIXME
	}
	
	return _base;
}

/**
 * Поиск блока по адресу
 */
static mem_chunk_t *get_chunk_by_address(void *ptr) {
	mem_chunk_t *chunk = base;
	
	while (chunk && (chunk != ptr+CHUNK_SIZE)) {
		chunk = chunk->next;
	}
	
	return chunk;
}

/**
 * Запрос на сдвиг break указателя
 */
static mem_chunk_t *request_space(size_t size) {
	mem_chunk_t *chunk;
	
	chunk = sbrk(0); // получаем текущий адрес break 
	if (sbrk(CHUNK_SIZE + size) == (void*)-1) {
		return NULL;
	}
	
	chunk->size = size;
	chunk->next = NULL;
	chunk->is_free = 1;
	
	if (last) {
		(*last)->next = chunk;
		last = &chunk;
	}
	
	return chunk;
}

void *malloc(size_t size) {
	size_t s;
	mem_chunk_t *chunk;
	int err_code;
	
#if DEBUG
	puts("Malloc called");
	fflush(stdout);
#endif

	size = _align(size);
	
	if (base) {
		chunk = find_chunk(size);
		if (!chunk) {
			// Подходящий блок не найден
			chunk = request_space(size);
			
			if (!chunk) {
				err_code = ENOMEM;
				goto err;
			}
		}
	} else {
		// Первое выделение памяти
		chunk = request_space(size);
		if (!chunk) {
			err_code = ENOMEM;
			goto err;
		}
		base = chunk;
		last = &base;
	}
#if DEBUG
	printf("chunk=%p base=%p last=%p ret=%p chunk_size=%lu\n", chunk, base, *last, CHUNK_MEM(chunk), CHUNK_SIZE);
#endif
	return CHUNK_MEM(chunk);

err:
	errno = err_code;
	return NULL;
}

void *calloc(size_t number, size_t size) {
	void *mem; 
	
#if DEBUG
	puts("Calloc called");
	fflush(stdout);
#endif
	
	size = _align(size * number);
	mem = malloc(size);
	
	if (mem) {
		memset(mem, 0, size);
	}
	
	return (void*)mem;
}

void free(void *ptr) {
	mem_chunk_t *chunk = base;
	
#if DEBUG
	puts("Free called");
	fflush(stdout);
#endif
	
	if (!ptr) {
		return;
	}
	 
	while (chunk) {
		if (chunk == MEM_CHUNK(ptr)) {
			chunk->is_free = 1;
		}
		chunk = chunk->next;
	}
}

void *realloc(void *ptr, size_t size) {
	mem_chunk_t *chunk = base;
	void *new_ptr = NULL;
	
#if DEBUG
	puts("Realloc called");
	fflush(stdout);
#endif
	
	size = _align(size);
	
	if (!ptr) {
		return malloc(size);
	} else {
		chunk = get_chunk_by_address(ptr);
		if (!chunk) {
			return NULL;
		}
		// если в этом блоке достаточно места
		if (chunk->size <= size) {
			return CHUNK_MEM(chunk);
			
		// если нет - выделяем новый кусок
		} else {
			new_ptr = malloc(size);
			if (!new_ptr) {
				return NULL;
			} else {
				memcpy(new_ptr, ptr, (MEM_CHUNK(ptr))->size); // TODO: проверить границы
				free(ptr);
				return new_ptr;
			}
		}
	}
}


