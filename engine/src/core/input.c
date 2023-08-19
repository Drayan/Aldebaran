#include "core/input.h"
#include "core/event.h"
#include "core/amemory.h"
#include "core/logger.h"
#include "input.h"

typedef struct keyboard_state
{
    b8 keys[256];
} keyboard_state;

typedef struct mouse_state
{
    i16 x;
    i16 y;
    b8 buttons[MOUSE_BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state
{
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

// Internal input state
static b8 is_initialized = FALSE;
static input_state state = {};

void initialize_inputs()
{
    azero_memory( &state, sizeof( input_state ) );
    is_initialized = TRUE;

    AINFO( "Input subsystem initialized." );
}

void shutdown_inputs()
{
    // TODO: Add shutdown routines when needed.
    is_initialized = FALSE;
}

void update_inputs( f64 delta_time )
{
    if( !is_initialized )
    {
        return;
    }

    // Copy current states to previous states.
    acopy_memory( &state.keyboard_previous, &state.keyboard_current, sizeof( keyboard_state ) );
    acopy_memory( &state.mouse_previous, &state.mouse_current, sizeof( mouse_state ) );
}

void input_process_key( keys key, b8 pressed )
{
    // Only handle this if the state actually changed.
    if( state.keyboard_current.keys[key] != pressed )
    {
        // Update internal state.
        state.keyboard_current.keys[key] = pressed;

        // Fire off an event for immediate processing.
        event_context context;
        context.data.u16[0] = key;
        event_fire( pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, context );

        // ADEBUG("Key %i state changed!", key);
    }
}

void input_process_mouse_button( mouse_buttons button, b8 pressed )
{
    // If the state changed, fire an event.
    if( state.mouse_current.buttons[button] != pressed )
    {
        state.mouse_current.buttons[button] = pressed;

        // Fire the event.
        event_context context;
        context.data.u16[0] = button;
        event_fire( pressed ? EVENT_CODE_MOUSE_BUTTON_PRESSED : EVENT_CODE_MOUSE_BUTTON_RELEASED,
            0,
            context );

        // ADEBUG("Mouse button %i state changed!", button);
    }
}

void input_process_mouse_move( i16 x, i16 y )
{
    // Only process if actually different.
    if( state.mouse_current.x != x || state.mouse_current.y != y )
    {
        // NOTE: Enable this for debugging.
        // ADEBUG( "Mouse pos: %i, %i!", x, y );

        // Update the internal state.
        state.mouse_current.x = x;
        state.mouse_current.y = y;

        // Fire the event.
        event_context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        event_fire( EVENT_CODE_MOUSE_MOVED, 0, context );
    }
}

void input_process_mouse_wheel( i8 z_delta )
{
    // NOTE: No internal state to update.

    // Fire the event.
    event_context context;
    context.data.u8[0] = z_delta;
    event_fire( EVENT_CODE_MOUSE_WHEEL, 0, context );
}

b8 input_is_key_down( keys key )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.keyboard_current.keys[key] == TRUE;
}

b8 input_is_key_up( keys key )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.keyboard_current.keys[key] == FALSE;
}

b8 input_was_key_down( keys key )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.keyboard_previous.keys[key] == TRUE;
}

b8 input_was_key_up( keys key )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.keyboard_previous.keys[key] == FALSE;
}

b8 input_is_mouse_button_down( mouse_buttons button )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.mouse_current.buttons[button] == TRUE;
}

b8 input_is_mouse_button_up( mouse_buttons button )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.mouse_current.buttons[button] == FALSE;
}

b8 input_was_mouse_button_down( mouse_buttons button )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.mouse_previous.buttons[button] == TRUE;
}

b8 input_was_mouse_button_up( mouse_buttons button )
{
    if( !is_initialized )
    {
        return FALSE;
    }

    return state.mouse_previous.buttons[button] == FALSE;
}

void input_get_mouse_position( i32 *x, i32 *y )
{
    if( !is_initialized )
    {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse_current.x;
    *y = state.mouse_current.y;
}

void input_get_previous_mouse_position( i32 *x, i32 *y )
{
    if( !is_initialized )
    {
        *x = 0;
        *y = 0;
        return;
    }

    *x = state.mouse_previous.x;
    *y = state.mouse_previous.y;
}
