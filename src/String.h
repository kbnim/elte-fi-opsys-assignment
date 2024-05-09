#ifndef String_H
#define String_H

#include <stdio.h>
#include <stdbool.h>

/* Opaque type definition of 'String'. */
typedef struct String String;

/* Constructor of a 'String' object. Returns 'NULL' upon failure. */
String* string_construct(const char* const str);

/* Destructor of a 'String' object. */
void string_destroy(String* str);

/* Returns the length of the string. Does not include the '\0' character. */
size_t string_get_length(const String* const string);

/* Returns the size of the string, which includes the special '\0' character. */
size_t string_get_size(const String* const string);

/* Returns the string in pure C-style form. */
const char* string_get_data(const String* const string);

/* Returns whether the string was used in the 'sprinkling' process */
bool string_get_is_used(const String* const string);

/* Sets the 'is_used' field to the entered value. */
void string_set_is_used(String* const string, bool value);

/* Compares two 'String' objects. It works the same was as 'strcmp' in C. */
int string_compare(const String *const left, const String *const right);

/* Specialised form of 'string_compare' returning a boolean value. */
bool string_are_equal(const String* const left, const String* const right);

/* 
  Specialised form of 'string_compare' returning a boolean value. 
  However, the 'right' parameter is a standarc C-style string. 
*/
bool string_are_equal_c(const String* const left, const char* const right);

/* Modifies the string object so that each alphabetical letter is capitalised. */
void string_transform_to_upper(String* const string);

/*
  Reads input of arbitrary length and creates a 'String' object out of it.
  If 'stdin' is passed in, the input is read from the console.
  Returns 'NULL' if and only if reading was unsuccessful.
 */
String* string_read_line(FILE* source);

#endif // String_H