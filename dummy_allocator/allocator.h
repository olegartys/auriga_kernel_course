#ifndef ALLOCATOR_H
#define ALLOCATOR_H

void *malloc(size_t size);

void *calloc(size_t number, size_t size);

void *realloc(void *ptr, size_t size);

void free(void *ptr);

#endif // ALLOCATOR_H
