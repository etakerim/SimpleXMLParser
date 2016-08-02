#include "vector.h"
#include <stdlib.h>
#include <string.h>

#define GROWTH_FACTOR 1.5
#define DEFAULT_COUNT_OF_ELEMENETS 8
#define MINIMUM_COUNT_OF_ELEMENTS 2

/* -------------------------------------------------------------------------- */

struct vector {
  size_t count;
  size_t element_size;
  size_t reserved_size;
  char *data;
  vector_deleter *deleter;
};

/* -------------------------------------------------------------------------- */
/* auxillary methods */

bool vector_realloc(Vector *vector, size_t new_count)
{
    const size_t new_size = new_count * vector->element_size;
    char *new_data = (char *) realloc(vector->data, new_size);
    if (!new_data) {
        return false;
    }

    vector->reserved_size = new_size;
    vector->data = new_data;
    return true;
}

void vector_call_deleter(Vector *vector, size_t first_index, size_t last_index)
{
    size_t i;
    for (i = first_index; i < last_index; ++i) {
        vector->deleter(vector_at(vector, i));
    }
}

void vector_call_deleter_all(Vector *vector)
{
    vector_call_deleter(vector, 0, vector_count(vector));
}

/* -------------------------------------------------------------------------- */
/* Contol */

Vector *vector_create(size_t count_elements, size_t size_of_element, vector_deleter *deleter)
{
    Vector *v = (Vector *) malloc(sizeof(Vector));
    if (v != NULL) {
        v->data = NULL;
        v->count = 0;
        v->element_size = size_of_element;
        v->deleter = deleter;

        if (count_elements < MINIMUM_COUNT_OF_ELEMENTS) {
            count_elements = DEFAULT_COUNT_OF_ELEMENETS;
        }

        if (size_of_element < 1 || !vector_realloc(v, count_elements)) {
            free(v);
            v = NULL;
        }
    }
    return v;
}

Vector *vector_create_copy(const Vector *vector)
{
    Vector *new_vector = vector_create(vector->reserved_size / vector->count,
                                        vector->element_size,
                                        vector->deleter);
    if (!new_vector) {
        return new_vector;
    }

    if (memcpy(vector->data,
               new_vector->data,
               new_vector->element_size * vector->count) == NULL) {
        vector_release(new_vector);
        new_vector = NULL;
        return new_vector;
    }

    new_vector->count = vector->count;
    return new_vector;
}

void vector_release(Vector *vector)
{
    if (vector->deleter != NULL) {
        vector_call_deleter_all(vector);
    }

    if (vector->reserved_size != 0) {
        free(vector->data);
    }

    free(vector);
}

bool vector_is_equals(Vector *vector1, Vector *vector2)
{
    const size_t size_vector1 = vector_size(vector1);
    if (size_vector1 != vector_size(vector2)) {
        return false;
    }

    return memcmp(vector1->data, vector2->data, size_vector1) == 0;
}

float vector_get_growth_factor()
{
    return GROWTH_FACTOR;
}

size_t vector_get_default_count_of_elements()
{
    return DEFAULT_COUNT_OF_ELEMENETS;
}

size_t vector_struct_size()
{
    return sizeof(Vector);
}

/* -------------------------------------------------------------------------- */
/* Element access */

void *vector_at(Vector *vector, size_t index)
{
    return vector->data + index * vector->element_size;
}

void *vector_front(Vector *vector)
{
    return vector->data;
}

void *vector_back(Vector *vector)
{
    return vector->data + (vector->count - 1) * vector->element_size;
}

void *vector_data(Vector *vector)
{
    return vector->data;
}

/* -------------------------------------------------------------------------- */
/* Iterators */

void *vector_begin(Vector *vector)
{
    return vector->data;
}

void *vector_end(Vector *vector)
{
    return vector->data + vector->element_size * vector->count;
}

void *vector_next(Vector *vector, void *i)
{
    return ((char *) i) + vector->element_size;
}

/* -------------------------------------------------------------------------- */
/* Capacity */

bool vector_empty(Vector *vector)
{
    return vector->count == 0;
}

size_t vector_count(const Vector *vector)
{
    return vector->count;
}

size_t vector_size(const Vector *vector)
{
    return vector->count * vector->element_size;
}

size_t vector_max_count(const Vector *vector)
{
    return vector->reserved_size / vector->element_size;
}

size_t vector_max_size(const Vector *vector)
{
    return vector->reserved_size;
}

bool vector_reserve_count(Vector *vector, size_t new_count)
{
    size_t new_size;
    if (new_count < vector->count) {
        return false;
    }

    new_size = vector->element_size * new_count;
    if (new_size == vector->reserved_size) {
        return true;
    }

    return vector_realloc(vector, new_count);
}

bool vector_reserve_size(Vector *vector, size_t new_size)
{
    return vector_reserve_count(vector, new_size / vector->element_size);
}

/* -------------------------------------------------------------------------- */
/* Modifiers */

void vector_clear(Vector *vector)
{
    if (vector->deleter != NULL) {
        vector_call_deleter_all(vector);
    }

    vector->count = 0;
}

bool vector_insert(Vector *vector, size_t index, const void *value)
{
    if (vector_max_count(vector) < vector->count + 1) {
        if (!vector_realloc(vector, vector_max_count(vector) * GROWTH_FACTOR)) {
            return false;
        }
    }

    if (!memmove(vector_at(vector, index + 1),
                 vector_at(vector, index),
                 vector->element_size * (vector->count - index))) {

        return false;
    }

    if (memcpy(vector_at(vector, index),
                        value,
                        vector->element_size) == NULL) {
        return false;
    }

    ++vector->count;
    return true;
}

bool vector_erase(Vector *vector, size_t index)
{
    if (vector->deleter != NULL) {
        vector->deleter(vector_at(vector, index));
    }

    if (!memmove(vector_at(vector, index),
                 vector_at(vector, index + 1),
                 vector->element_size * (vector->count - index))) {
      return false;
    }

    vector->count--;
    return true;
}

bool vector_erase_range(Vector *vector, size_t first_index, size_t last_index)
{
    if (vector->deleter != NULL) {
        vector_call_deleter(vector, first_index, last_index);
    }

    if (!memmove(vector_at(vector, first_index),
                 vector_at(vector, last_index),
                vector->element_size * (vector->count - last_index))) {
        return false;
    }

    vector->count -= last_index - first_index;
    return true;
}

bool vector_append(Vector *vector, const void *values, size_t count)
{
    const size_t count_new = count + vector_count(vector);

    if (vector_max_count(vector) < count_new) {
        size_t max_count_to_reserved = vector_max_count(vector) * GROWTH_FACTOR;
        while (count_new > max_count_to_reserved) {
            max_count_to_reserved *= GROWTH_FACTOR;
        }

        if (!vector_realloc(vector, max_count_to_reserved)) {
            return false;
        }
    }

    if (memcpy(vector->data + vector->count * vector->element_size,
                        values,
                        vector->element_size * count) == NULL) {
        return false;
    }

    vector->count = count_new;
    return true;
}

bool vector_push_back(Vector *vector, const void *value)
{
    if (!vector_append(vector, value, 1)) {
        return false;
    }

    return true;
}

bool vector_pop_back(Vector *vector)
{
    if (vector->deleter != NULL) {
        vector->deleter(vector_back(vector));
    }

    vector->count--;
    return true;
}

bool vector_replace(Vector *vector, size_t index, const void *value)
{
    if (vector->deleter != NULL) {
        vector->deleter(vector_at(vector, index));
    }

    return memcpy(vector_at(vector, index),
                  value,
                  vector->element_size) != NULL;
}

bool vector_replace_multiple(Vector *vector, size_t index, const void *values, size_t count)
{
    if (vector->deleter != NULL) {
        vector_call_deleter(vector, index, index + count);
    }

    return memcpy(vector_at(vector, index),
                  values,
                  vector->element_size * count) != NULL;
}
