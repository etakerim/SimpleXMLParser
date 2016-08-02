#ifndef VCVECTOR_H
#define VCVECTOR_H

#include <stdbool.h>
#include <stdio.h>

typedef struct vector Vector;
typedef void (vector_deleter)(void *);

/* ----------------------------------------------------------------------------
 * Control
 * ----------------------------------------------------------------------------
 */
/* Constructs an empty vector with an reserver size for count_elements. */
Vector *vector_create(size_t count_elements, size_t size_of_element,vector_deleter *deleter);

/* Constructs a copy of an existing vector. */
Vector *vector_create_copy(const Vector *vector);

/* Releases the vector */
void vector_release(Vector *vector);

/* Compares vector content */
bool vector_is_equals(Vector *vector1, Vector* vector2);

/* Returns constant value of the vector growth factor. */
float vector_get_growth_factor();

/* Returns constant value of the vector default count of elements. */
size_t vector_get_default_count_of_elements();

/* Returns constant value of the vector struct size. */
size_t vector_struct_size();

/* ----------------------------------------------------------------------------
 * Element access
 * ----------------------------------------------------------------------------
 */

/* Returns the item at index position in the vector. */
void *vector_at(Vector *vector, size_t index);

/* Returns the first item in the vector. */
void *vector_front(Vector *vector);

/* Returns the last item in the vector. */
void *vector_back(Vector *vector);

/* Returns a pointer to the data stored in the vector. The pointer can be used
to access and modify the items in the vector. */
void *vector_data(Vector *vector);

/* ----------------------------------------------------------------------------
 * Iterators
 * ----------------------------------------------------------------------------
 */

/* Returns a pointer to the first item in the vector. */
void *vector_begin(Vector *vector);

/* Returns a pointer to the imaginary item after the last item in the vector. */
void *vector_end(Vector *vector);

/* Returns a pointer to the next element of vector after 'i'. */
void *vector_next(Vector *vector, void* i);

/* ----------------------------------------------------------------------------
 * Capacity
 * ----------------------------------------------------------------------------
 */

/* Returns true if the vector is empty; otherwise returns false. */
bool vector_empty(Vector *vector);

/* Returns the number of elements in the vector. */
size_t vector_count(const Vector *vector);

/* Returns the size (in bytes) of occurrences of value in the vector. */
size_t vector_size(const Vector *vector);

/* Returns the maximum number of elements that the vector can hold. */
size_t vector_max_count(const Vector *vector);

/* Returns the maximum size (in bytes) that the vector can hold. */
size_t vector_max_size(const Vector *vector);

/* Resizes the container so that it contains n elements. */
bool vector_reserve_count(Vector *vector, size_t new_count);

/* Resizes the container so that it contains new_size / element_size elements.*/
bool vector_reserve_size(Vector *vector, size_t new_size);

/* ----------------------------------------------------------------------------
 * Modifiers
 * ----------------------------------------------------------------------------
 */

/* Removes all elements from the vector (without reallocation). */
void vector_clear(Vector *vector);

/* The container is extended by inserting a new element at position. */
bool vector_insert(Vector *vector, size_t index, const void *value);

/* Removes from the vector a single element by 'index' */
bool vector_erase(Vector *vector, size_t index);

/* Removes from the vector a range of elements '[first_index, last_index)'. */
bool vector_erase_range(Vector *vector, size_t first_index, size_t last_index);

/* Inserts multiple values at the end of the vector. */
bool vector_append(Vector *vector, const void *values, size_t count);

/* Inserts value at the end of the vector. */
bool vector_push_back(Vector *vector, const void *value);

/* Removes the last item in the vector. */
bool vector_pop_back(Vector *vector);

/* Replace value by index in the vector. */
bool vector_replace(Vector *vector, size_t index, const void *value);

/* Replace multiple values by index in the vector. */
bool vector_replace_multiple(Vector *vector, size_t index, const void *values, size_t count);

#endif /* VCVECTOR_H */
