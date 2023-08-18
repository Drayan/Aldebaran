#pragma once

#include "defines.h"

typedef enum memory_tag 
{
    // For temporary use. Should be assigned one of the below or have a new tag created.
    MEMORY_TAG_UNKNOWN,

    MEMORY_TAG_ARRAY,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,

    MEMORY_TAG_MAX_TAGS
} memory_tag;

AAPI void initialize_memory();
AAPI void shutdown_memory();

AAPI void* aallocate(u64 suze, memory_tag tag);
AAPI void afree(void* block, u64 size, memory_tag tag);
AAPI void* azero_memory(void* block, u64 size);
AAPI void* acopy_memory(void* dest, const void* source, u64 size);
AAPI void* aset_memory(void* dest, i32 value, u64 size);
AAPI char* get_memory_usage_str();