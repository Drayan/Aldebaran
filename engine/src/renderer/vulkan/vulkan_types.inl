#pragma once

#include "defines.h"

#include "core/asserts.h"

#include <vulkan/vulkan.h>

// Checks the given exprs against VK_SUCCESS.
#define VK_CHECK(expr)                              \
    {                                               \
        VkResult result = expr;                     \
        ATRACE("Returned VK_CHECK %i.", result);    \
        AASSERT(result == VK_SUCCESS);              \
    }

typedef struct vulkan_context
{
    VkInstance instance;
    VkAllocationCallbacks *allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} vulkan_context;
