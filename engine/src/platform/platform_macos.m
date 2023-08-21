#include "platform/platform.h"

#if defined(APLATFORM_APPLE)

#include "core/logger.h"
#include "core/event.h"
#include "core/input.h"
#include "core/amemory.h"

#include <mach/mach_time.h>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <QuartzCore/CAMetalLayer.h>

@class ApplicationDelegate;
@class WindowDelegate;
@class ContentView;

typedef struct macos_handle_info
{
    CAMetalLayer *layer;
} macos_handle_info;

typedef struct internal_state
{
    ApplicationDelegate *app_delegate;
    WindowDelegate *wnd_delegate;
    NSWindow *window;
    ContentView *view;
    macos_handle_info handle;
    b8 quit_flagged;
    u8 modifier_key_states;
} internal_state;

enum macos_modifier_keys
{
    MACOS_MODIFIER_KEY_LSHIFT = 0x01,
    MACOS_MODIFIER_KEY_RSHIFT = 0x02,
    MACOS_MODIFIER_KEY_LCTRL = 0x04,
    MACOS_MODIFIER_KEY_RCTRL = 0x08,
    MACOS_MODIFIER_KEY_LOPTION = 0x10,
    MACOS_MODIFIER_KEY_ROPTION = 0x20,
    MACOS_MODIFIER_KEY_LCOMMAND = 0x40,
    MACOS_MODIFIER_KEY_RCOMMAND = 0x80
} macos_modifier_keys;

static platform_state *state_ptr;

// Key translation
static keys translate_keycode(u32 ns_keycode);

// Modifier key handling
static void handle_modifier_keys(u32 ns_keycode, u32 modifier_flags);

@interface WindowDelegate : NSObject <NSWindowDelegate>
{
    platform_state *state;
}

- (instancetype)initWithState:(platform_state *)init_state;

@end // WindowDelegate

@interface ContentView : NSView <NSTextInputClient>
{
    NSWindow *window;
    NSTrackingArea *trackingArea;
    NSMutableAttributedString *markedText;
}

- (instancetype)initWithWindow:(NSWindow *)initWindow;

@end // ContentView

@implementation ContentView

- (instancetype)initWithWindow:(NSWindow *)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
    }

    return self;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    input_process_mouse_button(MOUSE_BUTTON_LEFT, true);
}

- (void)mouseDragged:(NSEvent *)event
{
    // Equivalent to moving the mouve for now.
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    input_process_mouse_button(MOUSE_BUTTON_LEFT, false);
}

- (void)mouseMoved:(NSEvent *)event
{
    const NSPoint pos = [event locationInWindow];
    internal_state *state = (internal_state *)state_ptr->internal_state;

    // Neew to invert Y on macOS, since origin is bottom-left.
    // Also need to scale the mouse position by the device pixel ratio so screen lookups are correct.
    NSSize window_size = state->handle.layer.drawableSize;
    i16 x = pos.x * state->handle.layer.contentsScale;
    i16 y = pos.y * state->handle.layer.contentsScale;

    input_process_mouse_move(x, y);
}

- (void)rightMouseDown:(NSEvent *)event
{
    input_process_mouse_button(MOUSE_BUTTON_RIGHT, true);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    // Equivalent to moving the mouse for now
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    input_process_mouse_button(MOUSE_BUTTON_RIGHT, false);
}

- (void)otherMouseDown:(NSEvent *)event
{
    // Interpreted as middle button.
    input_process_mouse_button(MOUSE_BUTTON_MIDDLE, true);
}

- (void)otherMouseDragged:(NSEvent *)event
{
    // Equivalent to moving the mouse for now
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    // Interpreted as middle button.
    input_process_mouse_button(MOUSE_BUTTON_MIDDLE, false);
}

// Handle modifier keys since they are only registered via modifier flags being set/unset.
- (void)flagsChanged:(NSEvent *)event
{
    handle_modifier_keys([event keyCode], [event modifierFlags]);
}

- (void)keyDown:(NSEvent *)event
{
    keys key = translate_keycode((u32)[event keyCode]);

    input_process_key(key, true);
}

- (void)keyUp:(NSEvent *)event
{
    keys key = translate_keycode((u32)[event keyCode]);

    input_process_key(key, false);
}

- (void)scrollWheel:(NSEvent *)event
{
    input_process_mouse_wheel((i8)[event scrollingDeltaY]);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
}

- (void)unmarkText
{
}

// Defines a constant for empty ranges in NSTextInputClient
static const NSRange EMPTY_RANGE = {NSNotFound, 0};

- (NSRange)selectedRange
{
    return EMPTY_RANGE;
}

- (NSRange)markedRange
{
    return EMPTY_RANGE;
}

- (BOOL)hasMarkedText
{
    return false;
}

- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
    return NSMakeRect(0, 0, 0, 0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

@end // ContentView

@interface ApplicationDelegate : NSObject <NSApplicationDelegate>
{
}

@end // ApplicationDelegate

@implementation ApplicationDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)NSNotification
{
    // Positing and empty event at start
    @autoreleasepool
    {
        NSEvent *event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                            location:NSMakePoint(0, 0)
                                       modifierFlags:0
                                           timestamp:0
                                        windowNumber:0
                                             context:nil
                                             subtype:0
                                               data1:0
                                               data2:0];
        [NSApp postEvent:event atStart:YES];
    } // autoreleasepool

    [NSApp stop:nil];
}

@end // ApplicationDelegate

/**
 * WindowDelegate implementation
 */
@implementation WindowDelegate

- (instancetype)initWithState:(platform_state *)init_state
{
    self = [super init];

    if (self != nil)
    {
        state = init_state;
        internal_state *int_state = (internal_state *)state_ptr->internal_state;
        int_state->quit_flagged = FALSE;
    }

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    internal_state *int_state = (internal_state *)state_ptr->internal_state;
    int_state->quit_flagged = TRUE;

    event_context context = {};
    event_fire(EVENT_CODE_APPLICATION_QUIT, 0, context);

    return YES;
}

@end // WindowDelegate

b8 platform_startup(platform_state *plat_state, const char *application_name, i32 x, i32 y, i32 width, i32 height)
{
    plat_state->internal_state = malloc(sizeof(internal_state));
    internal_state *state = (internal_state *)plat_state->internal_state;
    state_ptr = plat_state;

    @autoreleasepool
    {
        [NSApplication sharedApplication];

        // App delegate creation
        state->app_delegate = [[ApplicationDelegate alloc] init];
        if (!state->app_delegate)
        {
            AERROR("Failed to create application delegate.");
            return FALSE;
        }
        [NSApp setDelegate:state->app_delegate];

        // Window delegate creation
        state->wnd_delegate = [[WindowDelegate alloc] initWithState:state_ptr];
        if (!state->wnd_delegate)
        {
            AERROR("Failed to create window delegate.");
            return FALSE;
        }

        // Window creation
        state->window = [[NSWindow alloc]
            initWithContentRect:NSMakeRect(x, y, width, height)
                      styleMask:NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                          defer:NO];
        if (!state->window)
        {
            AERROR("Failed to create window.");
            return FALSE;
        }

        // View creation
        state->view = [[ContentView alloc] initWithWindow:state->window];
        [state->view setWantsLayer:YES];

        // Layer creation
        state->handle.layer = [CAMetalLayer layer];
        if (!state->handle.layer)
        {
            AERROR("Failed to create layer for view.");
            return FALSE;
        }

        // Setting window properties
        [state->window setLevel:NSNormalWindowLevel];
        [state->window setContentView:state->view];
        [state->window makeFirstResponder:state->view];
        [state->window setTitle:@(application_name)];
        [state->window setDelegate:state->wnd_delegate];
        [state->window setAcceptsMouseMovedEvents:YES];
        [state->window setRestorable:NO];
        [state->window setReleasedWhenClosed:NO];

        if (![[NSRunningApplication currentApplication] isFinishedLaunching])
            [NSApp run];

        // Making the app a proper UI app since we're unbundled
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Putting window in front on launch
        [NSApp activateIgnoringOtherApps:YES];
        [state->window makeKeyAndOrderFront:nil];

        // Handle content scaling for various fidelity displays (i.e. Retina)
        state->handle.layer.bounds = state->view.bounds;
        // It's imporrtant to set the drawableSize to the actual backing pixels. When rendering
        // fullscreen, we can skip the macOS compositor if the size matches the display size.
        state->handle.layer.drawableSize = [state->view convertSizeToBacking:state->view.bounds.size];

        // In its implementation of vkGetPhysicalDeviceSurfaceCapabilitiesKHR, MoltenVK takes into
        // consideration both the size (in points) of the bounds, and the contentsScale of the
        // CAMetalLayer from which the Vulkan surface was created.
        state->handle.layer.contentsScale = state->view.window.backingScaleFactor;
        ADEBUG("contentScale: %f", state->handle.layer.contentsScale);

        [state->view setLayer:state->handle.layer];

        // This is set to NO by default, but is also important to ensure we can bypass the compositor
        // in fullscreen mode
        state->handle.layer.opaque = YES;

        // Fire off a resize event to make sure the framebuffer is the right size.
        // Again, this should be the actual backing framebuffer size (taking into account pixel density).
        event_context context;
        context.data.u16[0] = (u16)state->handle.layer.drawableSize.width;
        context.data.u16[1] = (u16)state->handle.layer.drawableSize.height;
        event_fire(EVENT_CODE_RESIZED, 0, context);

        return TRUE;
    }
}

void platform_shutdown(platform_state *plat_state)
{
    if (state_ptr)
    {
        internal_state *state = (internal_state *)state_ptr->internal_state;

        @autoreleasepool
        {
            [state->window close];

            [state->window setDelegate:nil];
            [state->wnd_delegate release];

            [state->view release];
            state->view = nil;

            [state->window close];
            state->window = nil;

            [NSApp setDelegate:nil];
            [state->app_delegate release];
            state->app_delegate = nil;
        } // autoreleasepool
    }

    state_ptr = 0;
}

b8 platform_pump_message(platform_state *plat_state)
{
    if (state_ptr)
    {
        @autoreleasepool
        {
            NSEvent *event;

            for (;;)
            {
                event = [NSApp
                    nextEventMatchingMask:NSEventMaskAny
                                untilDate:[NSDate distantPast]
                                   inMode:NSDefaultRunLoopMode
                                  dequeue:YES];

                if (!event)
                    break;

                [NSApp sendEvent:event];
            }
        } // autoreleasepool

        internal_state *state = (internal_state *)plat_state->internal_state;
        return !state->quit_flagged;
    }

    return true;
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
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    const char *color_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", color_strings[color], message);
}

void platform_console_write_error(const char *message, u8 color)
{
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    const char *color_strings[] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;30"};
    printf("\033[%sm%s\033[0m", color_strings[color], message);
}

f64 platform_get_absolute_time()
{
    mach_timebase_info_data_t clock_timebase;
    mach_timebase_info(&clock_timebase);

    u64 mach_absolute = mach_absolute_time();

    u64 nanos = (f64)(mach_absolute * (u64)clock_timebase.numer) / (f64)clock_timebase.denom;
    return nanos / 1.0e9; // Convert to seconds;
}

void platform_sleep(u64 ms)
{
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;
    nanosleep(&ts, 0);
#else
    if (ms >= 1000)
    {
        sleep(ms / 1000);
    }

    usleep((ms % 1000) * 1000);
#endif
}

static keys translate_keycode(u32 ns_keycode)
{
    // https://boredzo.org/blog/wp-content/uploads/2007/05/IMTx-virtual-keycodes.pdf
    // https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
    switch (ns_keycode)
    {
    case 0x52:
        return KEY_NUMPAD0;
    case 0x53:
        return KEY_NUMPAD1;
    case 0x54:
        return KEY_NUMPAD2;
    case 0x55:
        return KEY_NUMPAD3;
    case 0x56:
        return KEY_NUMPAD4;
    case 0x57:
        return KEY_NUMPAD5;
    case 0x58:
        return KEY_NUMPAD6;
    case 0x59:
        return KEY_NUMPAD7;
    case 0x5B:
        return KEY_NUMPAD8;
    case 0x5C:
        return KEY_NUMPAD9;

        // case 0x12:
        //     return KEY_1;
        // case 0x13:
        //     return KEY_2;
        // case 0x14:
        //     return KEY_3;
        // case 0x15:
        //     return KEY_4;
        // case 0x17:
        //     return KEY_5;
        // case 0x16:
        //     return KEY_6;
        // case 0x1A:
        //     return KEY_7;
        // case 0x1C:
        //     return KEY_8;
        // case 0x19:
        //     return KEY_9;
        // case 0x1D:
        //     return KEY_0;

    case 0x00:
        return KEY_A;
    case 0x0B:
        return KEY_B;
    case 0x08:
        return KEY_C;
    case 0x02:
        return KEY_D;
    case 0x0E:
        return KEY_E;
    case 0x03:
        return KEY_F;
    case 0x05:
        return KEY_G;
    case 0x04:
        return KEY_H;
    case 0x22:
        return KEY_I;
    case 0x26:
        return KEY_J;
    case 0x28:
        return KEY_K;
    case 0x25:
        return KEY_L;
    case 0x2E:
        return KEY_M;
    case 0x2D:
        return KEY_N;
    case 0x1F:
        return KEY_O;
    case 0x23:
        return KEY_P;
    case 0x0C:
        return KEY_Q;
    case 0x0F:
        return KEY_R;
    case 0x01:
        return KEY_S;
    case 0x11:
        return KEY_T;
    case 0x20:
        return KEY_U;
    case 0x09:
        return KEY_V;
    case 0x0D:
        return KEY_W;
    case 0x07:
        return KEY_X;
    case 0x10:
        return KEY_Y;
    case 0x06:
        return KEY_Z;

    // case 0x27:
    //     return KEY_APOSTROPHE;
    // case 0x2A:
    //     return KEY_BACKSLASH;
    case 0x2B:
        return KEY_COMMA;
    // case 0x18:
    //     return KEY_EQUAL; // Equal/Plus
    case 0x32:
        return KEY_GRAVE;
    // case 0x21:
    //     return KEY_LBRACKET;
    case 0x1B:
        return KEY_MINUS;
    case 0x2F:
        return KEY_PERIOD;
    // case 0x1E:
    //     return KEY_RBRACKET;
    case 0x29:
        return KEY_SEMICOLON;
    case 0x2C:
        return KEY_SLASH;
    case 0x0A:
        return KEYS_MAX_KEYS; // ?

    case 0x33:
        return KEY_BACKSPACE;
    case 0x39:
        return KEY_CAPITAL;
    case 0x75:
        return KEY_DELETE;
    case 0x7D:
        return KEY_DOWN;
    case 0x77:
        return KEY_END;
    case 0x24:
        return KEY_ENTER;
    case 0x35:
        return KEY_ESCAPE;
    case 0x7A:
        return KEY_F1;
    case 0x78:
        return KEY_F2;
    case 0x63:
        return KEY_F3;
    case 0x76:
        return KEY_F4;
    case 0x60:
        return KEY_F5;
    case 0x61:
        return KEY_F6;
    case 0x62:
        return KEY_F7;
    case 0x64:
        return KEY_F8;
    case 0x65:
        return KEY_F9;
    case 0x6D:
        return KEY_F10;
    case 0x67:
        return KEY_F11;
    case 0x6F:
        return KEY_F12;
    case 0x69:
        return KEY_PRINT;
    case 0x6B:
        return KEY_F14;
    case 0x71:
        return KEY_F15;
    case 0x6A:
        return KEY_F16;
    case 0x40:
        return KEY_F17;
    case 0x4F:
        return KEY_F18;
    case 0x50:
        return KEY_F19;
    case 0x5A:
        return KEY_F20;
    case 0x73:
        return KEY_HOME;
    case 0x72:
        return KEY_INSERT;
    // case 0x7B:
    //     return KEY_LEFT;
    // case 0x3A:
    //     return KEY_LALT;
    case 0x3B:
        return KEY_LCONTROL;
    case 0x38:
        return KEY_LSHIFT;
    // case 0x37:
    //     return KEY_LSUPER;
    case 0x6E:
        return KEYS_MAX_KEYS; // Menu
    case 0x47:
        return KEY_NUMLOCK;
    case 0x79:
        return KEYS_MAX_KEYS; // Page down
    case 0x74:
        return KEYS_MAX_KEYS; // Page up
    case 0x7C:
        return KEY_RIGHT;
    // case 0x3D:
    //     return KEY_RALT;
    case 0x3E:
        return KEY_RCONTROL;
    case 0x3C:
        return KEY_RSHIFT;
    // case 0x36:
    //     return KEY_RSUPER;
    case 0x31:
        return KEY_SPACE;
    case 0x30:
        return KEY_TAB;
    case 0x7E:
        return KEY_UP;

    case 0x45:
        return KEY_ADD;
    case 0x41:
        return KEY_DECIMAL;
    case 0x4B:
        return KEY_DIVIDE;
    case 0x4C:
        return KEY_ENTER;
    case 0x51:
        return KEY_NUMPAD_EQUAL;
    case 0x43:
        return KEY_MULTIPLY;
    case 0x4E:
        return KEY_SUBTRACT;

    default:
        return KEYS_MAX_KEYS;
    }
}

// Bitmasks for left and right version of these keys.
#define MACOS_LSHIFT_MASK (1 << 1)
#define MACOS_RSHIFT_MASK (1 << 2)
#define MACOS_LCTRL_MASK (1 << 0)
#define MACOS_RCTRL_MASK (1 << 13)
#define MACOS_LCOMMAND_MASK (1 << 3)
#define MACOS_RCOMMAND_MASK (1 << 4)
#define MACOS_LALT_MASK (1 << 5)
#define MACOS_RALT_MASK (1 << 6)

static void handle_modifier_key(
    u32 ns_keycode,
    u32 ns_key_mask,
    u32 ns_l_keycode,
    u32 ns_r_keycode,
    u32 k_l_keycode,
    u32 k_r_keycode,
    u32 modifier_flags,
    u32 l_mod,
    u32 r_mod,
    u32 l_mask,
    u32 r_mask)
{
    internal_state *state = (internal_state *)state_ptr->internal_state;

    if (modifier_flags & ns_key_mask)
    {
        // Check left variant
        if (modifier_flags & l_mask)
        {
            if (!(state->modifier_key_states & l_mod))
            {
                state->modifier_key_states |= l_mod;

                // Report the keypress
                input_process_key(k_l_keycode, true);
            }
        }

        // Check right variant
        if (modifier_flags & r_mask)
        {
            if (!(state->modifier_key_states & r_mod))
            {
                state->modifier_key_states |= r_mod;

                // Report the keypress
                input_process_key(k_r_keycode, true);
            }
        }
    }
    else
    {
        if (ns_keycode == ns_l_keycode)
        {
            if (state->modifier_key_states & l_mod)
            {
                state->modifier_key_states &= ~(l_mod);

                // Report the release.
                input_process_key(k_l_keycode, false);
            }
        }

        if (ns_keycode == ns_r_keycode)
        {
            if (state->modifier_key_states & r_mod)
            {
                state->modifier_key_states &= ~(r_mod);

                // Report the release.
                input_process_key(k_r_keycode, false);
            }
        }
    }
}

static void handle_modifier_keys(u32 ns_keycode, u32 modifier_flags)
{
    // Shift
    handle_modifier_key(
        ns_keycode,
        NSEventModifierFlagShift,
        0x38,
        0x3C,
        KEY_LSHIFT,
        KEY_RSHIFT,
        modifier_flags,
        MACOS_MODIFIER_KEY_LSHIFT,
        MACOS_MODIFIER_KEY_RSHIFT,
        MACOS_LSHIFT_MASK,
        MACOS_RSHIFT_MASK);

    ATRACE("modifier flags keycode: %u", ns_keycode);

    // Ctrl
    handle_modifier_key(
        ns_keycode,
        NSEventModifierFlagControl,
        0x3B,
        0x3E,
        KEY_LCONTROL,
        KEY_RCONTROL,
        modifier_flags,
        MACOS_MODIFIER_KEY_LCTRL,
        MACOS_MODIFIER_KEY_RCTRL,
        MACOS_LCTRL_MASK,
        MACOS_RCTRL_MASK);

    // Alt/Option
    // handle_modifier_key(
    //     ns_keycode,
    //     NSEventModifierFlagOption,
    //     0x3A,
    //     0x3D,
    //     KEY_LALT,
    //     KEY_RALT,
    //     modifier_flags,
    //     MACOS_MODIFIER_KEY_LOPTION,
    //     MACOS_MODIFIER_KEY_ROPTION,
    //     MACOS_LALT_MASK,
    //     MACOS_RALT_MASK
    // );

    // Command/Super
    // handle_modifier_key(
    //     ns_keycode,
    //     NSEventModifierFlagCommand,
    //     0x37,
    //     0x36,
    //     KEY_LSUPER,
    //     KEY_RSUPER,
    //     modifier_flags,
    //     MACOS_MODIFIER_KEY_LCOMMAND,
    //     MACOS_MODIFIER_KEY_RCOMMAND,
    //     MACOS_LCOMMAND_MASK,
    //     MACOS_RCOMMAND_MASK
    // );

    // Caps lock - handled a bit differently than other keys.
    if (ns_keycode == 0x39)
    {
        if (modifier_flags & NSEventModifierFlagCapsLock)
        {
            // Report as a keypress. This notifies the system that caps lock
            // has been turned on.
            input_process_key(KEY_CAPITAL, true);
        }
        else
        {
            // Report as a release. This notifies the system that caps lock
            // has been turned off.
            input_process_key(KEY_CAPITAL, false);
        }
    }
}

#endif
