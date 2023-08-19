#include "application.h"
#include "game_types.h"

#include "logger.h"
#include "platform/platform.h"
#include "core/amemory.h"
#include "core/event.h"
#include "core/input.h"

typedef struct application_state
{
    game *game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;
} application_state;

static b8 initialized = FALSE;
static application_state app_state;

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);

b8 application_create( game *game_inst )
{
    if( initialized )
    {
        AERROR( "application_create called more than once." );
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // Initialize subsystems.
    initialize_logging();

    if( !initialize_events() )
    {
        AFATAL( "Event system failed initialization. Application cannot continue." );
        return FALSE;
    }

    // Setup system events listener.
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    initialize_inputs();

    // TODO: Remove this.
    AFATAL( "A test message: %f", 3.14f );
    AERROR( "A test message: %f", 3.14f );
    AWARN( "A test message: %f", 3.14f );
    AINFO( "A test message: %f", 3.14f );
    ADEBUG( "A test message: %f", 3.14f );
    ATRACE( "A test message: %f", 3.14f );

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if( !platform_startup( &app_state.platform,
        game_inst->app_config.name,
        game_inst->app_config.start_pos_x,
        game_inst->app_config.start_pos_y,
        game_inst->app_config.start_width,
        game_inst->app_config.start_height ) )
    {
        return FALSE;
    }

    // Initialize the game.
    if( !app_state.game_inst->initialize( app_state.game_inst ) )
    {
        AFATAL( "Game failed to initialize." );
        return FALSE;
    }

    app_state.game_inst->on_resize( app_state.game_inst, app_state.width, app_state.height );

    initialized = TRUE;

    return TRUE;
}

b8 application_run()
{
    AINFO( get_memory_usage_str() );

    while( app_state.is_running )
    {
        if( !platform_pump_message( &app_state.platform ) )
        {
            app_state.is_running = FALSE;
        }

        if( !app_state.is_suspended )
        {
            if( !app_state.game_inst->update( app_state.game_inst, ( f32 )0 ) )
            {
                AFATAL( "Game update failed, shutting down." );
                app_state.is_running = FALSE;
                break;
            }

            // Call the game's render routine.
            if( !app_state.game_inst->render( app_state.game_inst, ( f32 )0 ) )
            {
                AFATAL( "Game render failed, shutting down." );
                app_state.is_running = FALSE;
                break;
            }

            // NOTE: Input update/state copying should always be handled after any
            // input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before this
            // frame ends.
            update_inputs( ( f64 )0 );
        }
    }

    app_state.is_running = FALSE;

    shutdown_inputs();

    // Unregister system events.
    event_unregister(EVENT_CODE_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_CODE_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_CODE_KEY_RELEASED, 0, application_on_key);

    shutdown_events();

    platform_shutdown( &app_state.platform );
    shutdown_logging();

    return TRUE;
}

b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context)
{
    switch (code)
    {
    case EVENT_CODE_APPLICATION_QUIT:
        AINFO("EVENT_CODE_APPLICATION_QUIT received, shutting down.");
        app_state.is_running = FALSE;
        return TRUE;
    
    default:
        break;
    }

    return FALSE;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context)
{
    if(code == EVENT_CODE_KEY_PRESSED)
    {
        u16 key_code = context.data.u16[0];
        if(key_code == KEY_ESCAPE)
        {
            // NOTE: Technically firing an event to itself, but there may be other listeners.
            event_context context = {};
            event_fire(EVENT_CODE_APPLICATION_QUIT, 0, context);

            // Block anything else from processing this.
            return TRUE;
        }
        else if(key_code == KEY_A) 
        {
            // Example on checking for a key
            ADEBUG("Explicit - A key pressed!");
        }
        else
        {
            ADEBUG("'%c' key pressed in window.", key_code);
        }
    }
    else if(code == EVENT_CODE_KEY_RELEASED)
    {
        u16 key_code = context.data.u16[0];
        if(key_code == KEY_B)
        {
            // Example on checking for a key.
            ADEBUG("Explicit - B key released!");
        }
        else
        {
            ADEBUG("'%c' key released in window.", key_code);
        }
    }

    return FALSE;
}
