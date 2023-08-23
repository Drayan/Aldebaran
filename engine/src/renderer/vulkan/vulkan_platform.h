#pragma once

#include "defines.h"

b8 platform_create_vulkan_surface(struct platform_state* plat_state, struct vulkan_context* context);

/**
 * Appends the names of required extensions for this platform to
 * the names_darray, which should be created and passed in.
 */
void platform_get_required_vulkan_extension_names(const char*** names_darray);
u32 platform_get_required_vulkan_flags();