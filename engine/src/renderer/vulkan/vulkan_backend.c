#include "vulkan_backend.h"

#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_swapchain.h"
#include "vulkan_types.inl"

#include "core/astring.h"
#include "core/logger.h"

#include "core/containers/darray.h"

static vulkan_context context;

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

i32 find_memory_index(u32 type_filter, u32 property_flags);

b8 vulkan_renderer_backend_initialize(
    renderer_backend* backend,
    const char* application_name,
    struct platform_state* plat_state)
{
    // Function pointers
    context.find_memory_index = find_memory_index;

    // TODO: Create a custom allocator.
    context.allocator = 0;

    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    app_info.pEngineName = "Aldebaran Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);   // Generic surface extension.
    platform_get_required_vulkan_extension_names(&required_extensions); // Platform-specific extension(s).
#if defined(_DEBUG)
    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // Debug utilities.

    ADEBUG("Required extensions:");
    u32 length = darray_length(required_extensions);
    for (u32 i = 0; i < length; ++i)
    {
        ADEBUG(required_extensions[i]);
    }
#endif

    create_info.enabledExtensionCount = darray_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    // Validation layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

// If validation should be done, get a list of the required validation layers names
// and make sure they exist. Validation layers should only be enabled on non-release builds.
#if defined(_DEBUG)
    AINFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    required_validation_layer_names = darray_create(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    required_validation_layer_count = darray_length(required_validation_layer_names);

    // Obtain a list of available validation layers.
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available.
    for (u32 i = 0; i < required_validation_layer_count; ++i)
    {
        AINFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = FALSE;
        for (u32 j = 0; j < available_layer_count; ++j)
        {
            if (strings_equal(required_validation_layer_names[i], available_layers[j].layerName))
                ;
            {
                found = TRUE;
                AINFO("Found.");
                break;
            }
        }

        if (!found)
        {
            AFATAL("Required validation layer is missing : %s.", required_validation_layer_names[i]);
            return FALSE;
        }
    }
    AINFO("All required validation layers are present.");
#endif

    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    create_info.flags = platform_get_required_vulkan_flags();

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    AINFO("Vulkan Instance created.");

#if defined(_DEBUG)
    ADEBUG("Creating Vulkan debugger...");
    u32 log_severity
        = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info
        = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vk_debug_callback;
    debug_create_info.pUserData = 0;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        context.instance,
        "vkCreateDebugUtilsMessengerEXT");
    AASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    ADEBUG("Vulkan debugger created.");
#endif

    // Surface creation
    platform_create_vulkan_surface(plat_state, &context);

    // Device creation
    if (!vulkan_device_create(&context))
    {
        AERROR("Failed to create the device.");
        return FALSE;
    }

    // Swapchain
    vulkan_swapchain_create(&context, context.framebuffer_width, context.framebuffer_height, &context.swapchain);

    AINFO("Vulkan renderer initialized successfully.");

    darray_destroy(required_extensions);
    darray_destroy(required_validation_layer_names);

    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend* backend)
{
    ADEBUG("Destroying Vulkan swapchain...");
    vulkan_swapchain_destroy(&context, &context.swapchain);

    ADEBUG("Destroying Vulkan device...");
    vulkan_device_destroy(&context);

    ADEBUG("Destroying Vulkan surface...");
    if (context.surface)
    {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

#if defined(_DEBUG)
    ADEBUG("Destroying Vulkan debugger...");
    if (context.debug_messenger)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            context.instance,
            "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }
#endif

    ADEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_on_resize(renderer_backend* backend, u16 width, u16 height) {}

b8 vulkan_renderer_backend_begin_frame(renderer_backend* backend, f32 delta_time) { return TRUE; }

b8 vulkan_renderer_backend_end_frame(renderer_backend* backend, f32 delta_time) { return TRUE; }

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    switch (message_severity)
    {
    default:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        AERROR(callback_data->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        AWARN(callback_data->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        AINFO(callback_data->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        ATRACE(callback_data->pMessage);
        break;
    }

    return VK_FALSE;
}

i32 find_memory_index(u32 type_filter, u32 property_flags)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        // Check each memory type to see if its bit is set to 1.
        if (type_filter & (1 << i)
            && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags)
        {
            return i;
        }
    }

    AWARN("Unable to find suitable memory type!");
    return -1;
}
