#include "vulkan_backend.h"

#include "vulkan_types.inl"

#include "core/logger.h"

static vulkan_context context;

b8 vulkan_renderer_backend_initialize(renderer_backend *backend, const char *application_name, struct platform_state *plat_state)
{
    // TODO: Create a custom allocator.
    context.allocator = 0;

    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    app_info.pEngineName = "Aldebaran Engine";
    app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

    // NOTE: Starting from Vulkan 1.3.216, theses extensions MUST be enabled on OSX to being able
    // to initialize the instance. As I'll probably refactor these later, and I'll probably
    // have to make a system to enabled extensions on other platforms, I'm not going to differentiace
    // platform now.
    const char *extension_names[] = {VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
                                     VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = 2;
    create_info.ppEnabledExtensionNames = extension_names;
    create_info.enabledLayerCount = 0;
    create_info.ppEnabledLayerNames = 0;
    create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    VkResult result = vkCreateInstance(&create_info, context.allocator, &context.instance);
    if (result != VK_SUCCESS)
    {
        AERROR("vkCreateInstance failed with result: %i", result);
        return FALSE;
    }

    AINFO("Vulkan renderer initialized successfully.");

    return TRUE;
}

void vulkan_renderer_backend_shutdown(renderer_backend *backend)
{
}

void vulkan_renderer_backend_on_resize(renderer_backend *backend, u16 width, u16 height)
{
}

b8 vulkan_renderer_backend_begin_frame(renderer_backend *backend, f32 delta_time)
{
    return TRUE;
}

b8 vulkan_renderer_backend_end_frame(renderer_backend *backend, f32 delta_time)
{
    return TRUE;
}
