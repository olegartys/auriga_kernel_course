#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "allocator_priv.h"

static mem_chunk_t *base = NULL;

/**
 *  Выравнивание по границе 8 байт
 */
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
	chunk->is_free = 0;
	
	return chunk;
}

void *malloc(size_t size) {
	size_t s;
	mem_chunk_t *chunk;
	int err_code;
		
	if (size <= 0) {
		err_code = EINVAL;
		goto err;
	}
	
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
	}
	
	return chunk;

err:
	errno = err_code;
	return NULL;
}

void *calloc(size_t number, size_t size) {
	intptr_t *mem; 
	size_t alloc_size;
	int i;
	
	alloc_size = _align(size * number);
	mem = malloc(alloc_size);
	
	if (mem) {
		for (i = 0; i < alloc_size; ++i) {
			mem[i] = 0;
		}
	}
	
	return (void*)mem;
}

void free(void *ptr) {
	void *chunk = base;
	
	while (chunk) {
		if (chunk == ptr) {
			chunk->free = 1;
		}
		chunk = chunk->next;
	}
}

if (Pointer == nullptr) {
				return this->Alloc(Size);
			} else {
				block_meta_info block = get_block_by_address(block_list, Pointer);
				if (block.size >= Size) { // если места в блоке соотв. этому адресу ддостаточно
					return Pointer;
				}
				
				// если места недостаточно, выделяем память под новый кусок и копируем содержимое в неё
				void* new_ptr = this->Alloc(Size);
				if (new_ptr == nullptr) {
					return nullptr;
				} else {
					memcpy(new_ptr, Pointer, block.size);
					this->Free(Pointer);
					return new_ptr;
				}
			}

void *realloc(void *ptr, size_t size) {
	mem_chunk_t *chunk = base;
	
	if (!ptr) {
		return malloc(size);
	} else {
		
	}
}


