#ifndef Vector_H
#define Vector_H

#include <stddef.h>

#include "String.h"

/* Opaque type definition of 'Vector'. */
typedef struct Vector Vector;

/* Constructor for a 'Vector' object. Returns 'NULL' upon failure. */
Vector* vector_construct(void);

/* Destructor for a 'Vector' object. */
void vector_destroy(Vector* vector);

/* Returns the number of elements stored in the vector. */
size_t vector_get_size(const Vector* const vector);

/* Returns the number of strings that were used at some point in the vector. */
size_t vector_get_used_count(const Vector* const vector);

/*
  Returns the C-style form of the string at the specified index.
  A shortcut to calling 'string_get_data(vector_get_string_at(...))'.
*/
const char* vector_get_at(const Vector* const vector, size_t index);

/* Returns the 'String'-type object at the specified index. */
String* vector_get_string_at(const Vector* const vector, size_t index);

/*
  Changes the string stored at the specified index.
  If the index is out of range, it does nothing.
*/
void vector_set_at(Vector* vector, size_t index, String* const string);

/* Prints out each element of the vector in a formatted way. */
void vector_print(const Vector* const vector);

/* Appends a string to the end of the vector. Similar to 'push_back' in C++ */
void vector_append(Vector* vector, String* const string);

/*
  Removes the string at the specified index and shift each element to fill the gap.
  If the index is out of range, it does nothing.
*/
void vector_remove_at(Vector* vector, size_t index);

/* Sets the specified string as 'used'. */
void vector_set_used(Vector* vector, size_t index);

#endif // Vector_H