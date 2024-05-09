#include <stdio.h>
#include <stdlib.h>

#include "hdr/Vector.h"
#include "hdr/MemoryAllocation.h"

struct Vector
{
    String** data;
    size_t size;
    size_t capacity;
    size_t used_count;
};

/* STATIC FUNCTIONS */

static void vector_double_capacity(Vector* vector)
{
    vector->data = DOUBLE_ARRAY(vector->data, vector->capacity, String*);
    vector->capacity = 2 * vector->capacity;

    for (size_t i = vector->size; i < vector->capacity; i++)
    {
        vector->data[i] = NULL;
    }
}

/* NON-STATIC FUNCTIONS */

Vector* vector_construct(void)
{
    Vector* vector = ALLOCATE(Vector);

    if (vector == NULL)
    {
        return NULL;
    }

    vector->data = ALLOCATE_ARRAY(String*, 1);

    if (vector->data == NULL)
    {
        return NULL;
    }

    vector->capacity = 1;
    vector->size = 0;
    vector->used_count = 0;
    return vector;
}

void vector_destroy(Vector* vector)
{
    if (vector != NULL)
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        free(vector);
    }

    vector = NULL;
}

size_t vector_get_size(const Vector* const vector)
{
    return vector->size;
}

size_t vector_get_used_count(const Vector *const vector)
{
    return vector->used_count;
}

void vector_print(const Vector* const vector)
{
    if (vector->size == 0)
    {
        puts("(empty)");
    }
    else
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            printf("[%lu] %s", i + 1, vector_get_at(vector, i));

            if (string_get_is_used(vector->data[i]))
            {
                printf(" (USED)");
            }

            printf("\n");
        }
    }
}

void vector_append(Vector* vector, String* const string)
{
    if (vector == NULL)
        return;

    if (vector->capacity == vector->size)
    {
        vector_double_capacity(vector);
    }

    vector->data[vector->size] = string;
    vector->size++;
}

const char* vector_get_at(const Vector* const vector, size_t index)
{
    if (index >= vector->size)
    {
        return NULL;
    }

    return string_get_data(vector->data[index]);
}

String* vector_get_string_at(const Vector* const vector, size_t index)
{
    if (index >= vector->size)
    {
        return NULL;
    }

    return vector->data[index];
}

void vector_set_at(Vector *vector, size_t index, String* const string)
{
    if (index >= vector->size)
    {
        return;
    }

    string_destroy(vector->data[index]);
    vector->data[index] = string;
}

/*
  Note: I'm absolutely sure there's a far more efficient algorithm
  that does not require to deep-copy or needlessly allocate memory
  on the heap. The primary reason of choosing this solution was that
  it proved to be the most memory-safe and avoided memory leaks.
*/
void vector_remove_at(Vector* vector, size_t index)
{
    if (index >= vector->size)
    {
        return;
    }

    // if the removal does not cause the vector to become empty
    if (vector->size - 1 != 0)
    {
        bool str_is_used = string_get_is_used(vector->data[index]);
        size_t vec_used_count = vector->used_count;
        String** string_arr = ALLOCATE_ARRAY(String*, vector->size - 1);
        size_t arr_index = 0;

        // deep-copying strings that are supposed to be preserved
        for (size_t i = 0; i < vector->size; i++)
        {
            if (i != index)
            {
                string_arr[arr_index] = string_construct(string_get_data(vector->data[i]));
                string_set_is_used(string_arr[arr_index], string_get_is_used(vector->data[i]));
                arr_index++;
            }
        }

        // deallocating original array
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        vector->data = string_arr;
        vector->size = arr_index;
        vector->capacity = arr_index;
        vector->used_count = (size_t)(str_is_used ? vec_used_count - 1 : vec_used_count);
    }
    else // avoiding calloc()-ing memory of 0 bytes
    {
        for (size_t i = 0; i < vector->size; i++)
        {
            string_destroy(vector->data[i]);
        }

        free(vector->data);
        vector->data = ALLOCATE_ARRAY(String*, 1);
        vector->size = 0;
        vector->capacity = 1;
    }
}

void vector_set_used(Vector* vector, size_t index)
{
    if (!string_get_is_used(vector->data[index]))
    {
        string_set_is_used(vector->data[index], true);
        vector->used_count++;
    }
}