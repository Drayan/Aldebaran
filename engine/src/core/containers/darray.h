#pragma once

#include "defines.h"

// Memory layout
// u64 capacity = number of elements that can be held.
// u64 length = number of elements currently contained.
// u64 stride = size of each element in bytes.
// void* elements

enum
{
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

AAPI void* _darray_create(u64 length, u64 stride);
AAPI void _darray_destroy(void* array);

AAPI u64 _darray_field_get(void* array, u64 field);
AAPI void _darray_field_set(void* array, u64 field, u64 value);

AAPI void* _darray_resize(void* array);

AAPI void* _darray_push(void* array, const void* value_ptr);
AAPI void _darray_pop(void* array, void* dest);

AAPI void* _darray_pop_at(void* array, u64 index, void* dest);
AAPI void* _darray_insert_at(void* array, u64 index, void* value_ptr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

/**
 * Create a dynamic array of a certain type.
 * @param type The type of elements that will be stored.
 * @return A pointer to the created dynamic array.
*/
#define darray_create(type) \
    _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type))

/**
 * Reserve a dynamic array of the passed capacity.
 * Be aware that this function doesn't set the length of the array. Therefore if you
 * assign values to this array elements, you should call darray_length_set after that.
 * @param type The type of elements that will be stored.
 * @param capacity The number of elements this array can initially store.
 * @return A pointer to the created dynamic array.
*/
#define darray_reserve(type, capacity) \
    _darray_create(capacity, sizeof(type))

/**
 * Destroy the dynamic array, reclaiming the memory.
*/
#define darray_destroy(array) _darray_destroy(array)

/**
 * Add an element at the end of the array and resize it if needed.
 * @param array A pointer to the dynamic array.
 * @param value The value to push into.
*/
#define darray_push(array, value)               \
    {                                           \
        __auto_type temp = value;             \
        array = _darray_push(array, &temp);     \
    }
// NOTE: could use __auto_type for temp above, but intellisense for VSCode falgs it
// as unknown type. typeof() seems to work just fine. Both are GNU extensions.

/**
 * Remove the element at the end of the array and return it to the user.
 * @param array A pointer to the dynamic array.
 * @param value_ptr A pointer to where to store the returned element.
*/
#define darray_pop(array, value_ptr) \
    _darray_pop(array, value_ptr)

/**
 * Insert an element at a specific index into a dynamic array.
 * Resize the array if needed.
 * @param array A pointer to the dynamic array.
 * @param index The index where the element will be inserted.
 * @param value The element to insert into.
*/
#define darray_insert_at(array, index, value)               \
    {                                                       \
        __auto_type temp = value;                           \
        array = _darray_insert_at(array, index, &temp);     \
    }

/**
 * Remove an element at a specific index from a dynamic array and return it.
 * @param array A pointer to the dynamic array.
 * @param index The index where the element will be removed.
 * @param value_ptr A pointer where the removed element will be returned.
*/
#define darray_pop_at(array, index, value_ptr) \
    _darray_pop_at(array, index, value_ptr)

/**
 * Clear every elements of a dynamic array.
 * @param array A pointer to the dynamic array.
*/
#define darray_clear(array) \
    _darray_field_set(array, DARRAY_LENGTH, 0)

/**
 * Retrieve the capacity of a dynamic array.
 * @param array A pointer to the dynamic array.
 * @return The capacity of the dynamic array pointed to.
*/
#define darray_capacity(array) \
    _darray_field_get(array, DARRAY_CAPACITY)

/**
 * Retrive the length of a dynamic array.
 * @param array A pointer to the dynamic array.
 * @return The length of the dynamic array pointed to.
*/
#define darray_length(array) \
    _darray_field_get(array, DARRAY_LENGTH)

/**
 * Retrieve the stride of a dynamic array.
 * @param array A pointer to the dynamic array.
 * @return The stride of the dynamic array pointed to.
*/
#define darray_stride(array) \
    _darray_field_get(array, DARRAY_STRIDE)

/**
 * Set the length of a dynamic array. Usefull when you want to create a
 * dynamic array of multiple elements directly (avoiding resizing).
 * @param array A pointer to the dynamic array.
 * @param value The new length of this dynamic array.
*/
#define darray_length_set(array, value) \
    _darray_field_set(array, DARRAY_LENGTH, value)
