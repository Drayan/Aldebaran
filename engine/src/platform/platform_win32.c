#include "platform/platform.h"

#if APLATFORM_WINDOWS

#include "core/logger.h"
#include "core/input.h"

#include "core/containers/darray.h"

#include <windows.h>
#include <windowsx.h> // Param input extraction
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.inl"

typedef struct internal_state
{
    HINSTANCE h_instance;
    HWND hwnd;
    VkSurfaceKHR surface;
} internal_state;

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(platform_state *plat_state, const char *application_name, i32 x, i32 y, i32 width, i32 height)
{
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = (internal_state *)plat_state->internal_state;

    state->h_instance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = "aldebaran_window_class";

    if (!RegisterClassA(&wc))
    {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Create window
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    // Obtain the size of the border.
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, the border rectangle is negative.
    window_x += border_rect.right - border_rect.left;
    window_y += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(window_ex_style, "aldebaran_window_class", application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state->h_instance, 0);

    if (handle == 0)
    {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        AFATAL("Window creation failed!");
        return FALSE;
    }
    else
    {
        state->hwnd = handle;
    }

    // Show the window
    b32 should_activate = 1; // TODO: If the window should not accept input, this should be false.
    i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE;
    ShowWindow(state->hwnd, show_window_command_flags);

    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return TRUE;
}

void platform_shutdown(platform_state *plat_state)
{
    // Simply cold-cast to the known type.
    internal_state *state = (internal_state *)plat_state->internal_state;

    if (state->hwnd)
    {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platform_pump_message(platform_state *plat_state)
{
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void *platform_allocate(u64 size, b8 aligned)
{
    return malloc(size);
}

void platform_free(void *block, b8 aligned)
{
    free(block);
}

void *platform_zero_memory(void *block, u64 size)
{
    return memset(block, 0, size);
}

void *platform_copy_memory(void *dest, const void *source, u64 size)
{
    return memcpy(dest, source, size);
}

void *platform_set_memory(void *dest, i32 value, u64 size)
{
    return memset(dest, value, size);
}

void platform_console_write(const char *message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
    OutputDebugString(message);
}

void platform_console_write_error(const char *message, u8 color)
{
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[color]);

    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(console_handle, message, (DWORD)length, number_written, 0);
    OutputDebugString(message);
}

f64 platform_get_absolute_time()
{
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);

    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms)
{
    Sleep(ms);
}

void platform_get_required_vulkan_extension_names(const char ***names_darray)
{
    darray_push(*names_darray, &"VK_KHR_win32_surface");
}

u32 platform_get_required_vulkan_flags()
{
    return 0;
}

b8 platform_create_vulkan_surface(platform_state *plat_state, vulkan_context *context)
{
    // Simply cold-cast to the known type.
    internal_state* state = (internal_state*)plat_state->internal_state;

    VkWin32SurfaceCreateInfoKHR create_info = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    create_info.hinstance = state->h_instance;
    create_info.hwnd = state->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(context->instance, &create_info, context->allocator, &state->surface);
    if(result != VK_SUCCESS)
    {
        AFATAL("Vulkan surface creation failed.");
        return FALSE;
    }

    context->surface = state->surface;
    return TRUE;
}

LRESULT win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param)
{
    switch (msg)
    {
    case WM_ERASEBKGND:
        // Notify the OS that erasing will be handled by the application to prevent flicker.
        return 1;

    case WM_CLOSE:
        // TODO: Fire an event for the application to quit.
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
    {
        // Get the updated size.
        // RECT r;
        // GetClientRect(hwnd, &r);
        // u32 width = r.right - r.left;
        // u32 height = r.bottom - r.top;

        // TODO: Fire an event for window resize.
    }
    break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        // Key pressed/released
        b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
        keys key = (u16)w_param;

        // Pass to the input subsystem for processing.
        input_process_key(key, pressed);
    }
    break;

    case WM_MOUSEMOVE:
    {
        // Mouse move
        i32 x_position = GET_X_LPARAM(l_param);
        i32 y_position = GET_Y_LPARAM(l_param);

        // Pass to the input subsystem for processing.
        input_process_mouse_move(x_position, y_position);
    }
    break;

    case WM_MOUSEWHEEL:
    {
        u32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
        if (z_delta != 0)
        {
            // Flatten the input to an OS-independent (-1, 1)
            z_delta = (z_delta < 0) ? -1 : 1;
            input_process_mouse_wheel(z_delta);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    {
        b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
        mouse_buttons button = MOUSE_BUTTON_MAX_BUTTONS;
        switch (msg)
        {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            button = MOUSE_BUTTON_LEFT;
            break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            button = MOUSE_BUTTON_MIDDLE;
            break;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
            button = MOUSE_BUTTON_RIGHT;
            break;
        }

        if (button != MOUSE_BUTTON_MAX_BUTTONS)
        {
            input_process_mouse_button(button, pressed);
        }
    }
    break;

    default:
        // Anything else.
        break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif
