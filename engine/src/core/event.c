#include "event.h"

#include "amemory.h"
#include "logger.h"
#include "containers/darray.h"

typedef struct registered_event
{
    void *listener;
    PFN_on_event callback;
} registered_event;

typedef struct event_code_entry
{
    registered_event *events;
} event_code_entry;

// This should be more than enough codes...
#define MAX_MESSAGE_CODES 16384

// State structure.
typedef struct event_system_state
{
    // Lookup table for event codes.
    event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

/**
 * Event system internal state.
 */
static b8 is_initialized = FALSE;
static event_system_state state;

b8 initialize_events()
{
    if (is_initialized)
    {
        return FALSE;
    }

    is_initialized = FALSE;
    azero_memory(&state, sizeof(state));

    is_initialized = TRUE;

    return TRUE;
}

void shutdown_events()
{
    // Free the events arrays. And objects pointed to should be destroyed on their own.
    for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i)
    {
        if (state.registered[i].events != 0)
        {
            darray_destroy(state.registered[i].events);
            state.registered[i].events = 0;
        }
    }
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event)
{
    if (is_initialized == FALSE)
    {
        AERROR("event_register for event %i is called before initialization.", code);
        return FALSE;
    }

    if (state.registered[code].events == 0)
    {
        state.registered[code].events = darray_create(registered_event);
    }

    u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        if (state.registered[code].events[i].listener == listener)
        {
            // TODO: warn
            AWARN("Trying to register an already registered listener for event %i.", code);
            return FALSE;
        }
    }

    // If at this point, no duplicate was found. Proceed with registration.
    registered_event event;
    event.listener = listener;
    event.callback = on_event;
    darray_push(state.registered[code].events, event);

    return TRUE;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event)
{
    if (is_initialized == FALSE)
    {
        AERROR("event_unregister for event %i is called before initialization.", code);
        return FALSE;
    }

    // On nothing is registered for the code, boot out.
    if (state.registered[code].events == 0)
    {
        // TODO: warn
        AWARN("An event %i is fired but no listeners was registered.", code);
        return FALSE;
    }

    u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state.registered[code].events[i];
        if (e.listener == listener && e.callback == on_event)
        {
            // Found one, remote it.
            registered_event popped_event;
            darray_pop_at(state.registered[code].events, i, &popped_event);

            return TRUE;
        }
    }

    // Not found.
    return FALSE;
}

b8 event_fire(u16 code, void *sender, event_context data)
{
    if (is_initialized == FALSE)
    {
        AERROR("An event %i is fired before the event subsystem was initialized.", code);
        return FALSE;
    }

    // If nothing is registered for the code, boot out.
    if (state.registered[code].events == 0)
    {
        return FALSE;
    }

    u64 registered_count = darray_length(state.registered[code].events);
    for (u64 i = 0; i < registered_count; ++i)
    {
        registered_event e = state.registered[code].events[i];
        if (e.callback(code, sender, e.listener, data))
        {
            // Message had been handled, do not send to other listeners.
            return TRUE;
        }
    }

    // Not found.
    return FALSE;
}
