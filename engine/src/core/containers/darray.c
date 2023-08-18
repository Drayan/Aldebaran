#include "darray.h"

#include "core/logger.h"
#include "core/amemory.h"

void *_darray_create(u64 length, u64 stride)
{
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = aallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;

    return (void*)(new_array + DARRAY_FIELD_LENGTH);
}

void _darray_destroy(void *array)
{
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];

    afree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void *array, u64 field)
{
    if(field < 0 || field >= DARRAY_FIELD_LENGTH)
        AWARN("_darray_field_get called with an invalid field argument of %i!", field);

    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void *array, u64 field, u64 value)
{
    if(field < 0 || field >= DARRAY_FIELD_LENGTH)
        AWARN("_darray_field_set called with an invalid field argument of %i!", field);

    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void *_darray_resize(void *array)
{
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    void* temp = _darray_create(DARRAY_RESIZE_FACTOR * darray_capacity(array), stride);

    acopy_memory(temp, array, length * stride);

    darray_length_set(array, length);

    return temp;
}

void * _darray_push(void * array, const void * value_ptr)
{
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    if(length >= darray_capacity(array))
    {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    acopy_memory((void*)addr, value_ptr, stride);
    darray_length_set(array, length + 1);

    return array;
}

void _darray_pop(void * array, void * dest)
{
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    acopy_memory(dest, (void*)addr, stride);
    darray_length_set(array, length - 1);
}

AAPI void * _darray_pop_at(void * array, u64 index, void * dest)
{
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length)
    {
        AERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }

    u64 addr = (u64)array;
    acopy_memory(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if(index != length - 1)
    {
        acopy_memory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index)
        );
    }

    darray_length_set(array, length - 1);
    return array;
}

void * _darray_insert_at(void * array, u64 index, void * value_ptr)
{
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if(index >= length)
    {
        AERROR("Index outside the bounds of this array! Length: %i, index: %i", length, index);
        return array;
    }

    if(length >= darray_capacity(array))
    {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;

    // If not on the last element, copy the read half of the array outward.
    if(index != length - 1) 
    {
        acopy_memory(
            (void*)(addr + (index + 1) * stride),
            (void*)(addr + (index * stride)),
            stride * (length - index)
        );
    }

    // Set the value at the index.
    acopy_memory((void*)(addr + (index * stride)), value_ptr, stride);

    darray_length_set(array, length + 1);
    return array;
}
