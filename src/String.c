#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "String.h"
#include "MemoryAllocation.h"

struct String
{
    char* data;
    size_t length;
    size_t size;
    bool is_used;
};

String* string_construct(const char* const str)
{
    if (str == NULL)
    {
        return string_construct("");
    }

    String* string = ALLOCATE(String);

    if (string == NULL)
    {
        return NULL;
    }

    string->length = strlen(str);
    string->size = string->length + 1;
    string->data = ALLOCATE_ARRAY(char, string->size);
    string->is_used = false;
    strncpy(string->data, str, string->size);
    return string;
}

void string_destroy(String* str)
{
    if (str != NULL)
    {
        free(str->data);
        free(str);
    }

    str = NULL;
}

size_t string_get_length(const String* const string)
{
    return string->length;
}

size_t string_get_size(const String *const string)
{
    return string->size;
}

const char* string_get_data(const String* const string)
{
    return string->data;
}

bool string_get_is_used(const String* const string)
{
    return string->is_used;
}

void string_set_is_used(String *const string, bool value)
{
    string->is_used = value;
}

int string_compare(const String* const left, const String* const right)
{
    return strcmp(left->data, right->data);
}

String* string_read_line(FILE* source)
{
    char* buffer = ALLOCATE_ARRAY(char, DEFAULT_BUFFER_SIZE);
    size_t size = 0;
    size_t capacity = DEFAULT_BUFFER_SIZE;
    int character = fgetc(source);

    while (character != '\n' && character != EOF)
    {
        if (size == capacity)
        {
            buffer = DOUBLE_ARRAY(buffer, capacity, char);
            capacity *= 2;
        }

        buffer[size] = (char)character;
        size++;

        character = fgetc(source);
    }

    if (size == 0 && character == EOF)
    {
        free(buffer);
        return string_construct("");
    }

    buffer[size] = '\0';
    char* trimmed_buffer = ALLOCATE_ARRAY(char, size + 1);
    strncpy(trimmed_buffer, buffer, size + 1);
    trimmed_buffer[size] = '\0';

    String* string = string_construct(trimmed_buffer);
    free(buffer);
    free(trimmed_buffer);
    return string;
}

bool string_are_equal(const String* const left, const String* const right)
{
    return string_compare(left, right) == 0;
}

bool string_are_equal_c(const String* const left, const char* const right)
{
    return strncmp(left->data, right, left->size) == 0;
}

void string_transform_to_upper(String* const string)
{
    for (size_t i = 0; i < string->length; i++)
    {
        if (isalpha(string->data[i]))
        {
            string->data[i] = toupper(string->data[i]);
        }
    }
}