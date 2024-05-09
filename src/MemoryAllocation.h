#ifndef MemoryAllocation_H
#define MemoryAllocation_H

#define DEFAULT_BUFFER_SIZE 32

#define ALLOCATE(TYPE) (TYPE *)calloc(1, sizeof(TYPE))
#define ALLOCATE_ARRAY(TYPE, size) (TYPE *)calloc(size, sizeof(TYPE))
#define DOUBLE_ARRAY(array, capacity, TYPE) (TYPE *)realloc((array), (capacity) * (2) * sizeof(TYPE))

#endif // MemoryAllocation_H