#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "allocator_priv.h"

pthread_mutex_t mem_mutex;
static mem_chunk_t *base = NULL;

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
	
	while (chunk && (chunk != MEM_CHUNK(ptr))) {
		chunk = chunk->next;
	}
	
	return chunk;
}

/**
 * Запрос на сдвиг break указателя
 */
static mem_chunk_t *request_space(size_t size) {
	mem_chunk_t *chunk;
	mem_chunk_t *_base;	

	chunk = sbrk(0); // получаем текущий адрес break 
	if (sbrk(CHUNK_SIZE + size) == (void*)-1) {
		return NULL;
	}

	chunk->size = size;
	chunk->next = NULL;
	chunk->is_free = 0;

	// Добавляем элемент в список 
	// FIXME: O(n)
	if (base != NULL) {
		_base = base;
		while (_base->next != NULL) {
			_base = _base->next;
		}

		_base->next = chunk;
	} else {
		base = chunk;
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

	pthread_mutex_lock(&mem_mutex);

	size = _align(size);
	
	chunk = find_chunk(size);
	if (!chunk) {
		// Подходящий блок не найден
		chunk = request_space(size);
		
		if (!chunk) {
			err_code = ENOMEM;
			goto err;
		}
	}

#if DEBUG
	printf("chunk=%p base=%p ret=%p chunk_size=%lu\n", chunk, base, CHUNK_MEM(chunk), CHUNK_SIZE);
#endif
	
	pthread_mutex_unlock(&mem_mutex);
	return CHUNK_MEM(chunk);

err:
	errno = err_code;
	pthread_mutex_unlock(&mem_mutex);
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
	 
	pthread_mutex_lock(&mem_mutex);
	while (chunk) {
		if (chunk == MEM_CHUNK(ptr)) {
			chunk->is_free = 1;
			break;
		}
		chunk = chunk->next;
	}
	pthread_mutex_unlock(&mem_mutex);
}

void *realloc(void *ptr, size_t size) {
	mem_chunk_t *chunk = base;
	void *new_ptr = NULL;
	
#if DEBUG
	puts("Realloc called");
	fflush(stdout);
#endif
	
	pthread_mutex_lock(&mem_mutex);
	size = _align(size);
	
	if (!ptr) {
		pthread_mutex_unlock(&mem_mutex);
		return malloc(size);
	} else {
		chunk = get_chunk_by_address(ptr);
		if (!chunk) {
			pthread_mutex_unlock(&mem_mutex);
			return NULL;
		}
		// если в этом блоке достаточно места
		if (chunk->size >= size) {
			pthread_mutex_unlock(&mem_mutex);
			return CHUNK_MEM(chunk);
			
		// если нет - выделяем новый кусок
		} else {
			pthread_mutex_unlock(&mem_mutex);
			new_ptr = malloc(size);
			pthread_mutex_lock(&mem_mutex);
			if (!new_ptr) {
				pthread_mutex_unlock(&mem_mutex);
				return NULL;
			} else {
				memcpy(new_ptr, ptr, (MEM_CHUNK(ptr))->size); // TODO: проверить границы
				pthread_mutex_unlock(&mem_mutex);
				free(ptr);
				return new_ptr;
			}
		}
	}
}


