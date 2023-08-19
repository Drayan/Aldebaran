#include "application.h"
#include "game_types.h"

#include "logger.h"
#include "platform/platform.h"
#include "core/amemory.h"
#include "core/event.h"

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

b8 application_create(game *game_inst)
{
    if (initialized)
    {
        AERROR("application_create called more than once.");
        return FALSE;
    }

    app_state.game_inst = game_inst;

    // Initialize subsystems.
    initialize_logging();

    if (!initialize_events())
    {
        AFATAL("Event system failed initialization. Application cannot continue.");
        return FALSE;
    }

    // TODO: Remove this.
    AFATAL("A test message: %f", 3.14f);
    AERROR("A test message: %f", 3.14f);
    AWARN("A test message: %f", 3.14f);
    AINFO("A test message: %f", 3.14f);
    ADEBUG("A test message: %f", 3.14f);
    ATRACE("A test message: %f", 3.14f);

    app_state.is_running = TRUE;
    app_state.is_suspended = FALSE;

    if (!platform_startup(&app_state.platform,
                          game_inst->app_config.name,
                          game_inst->app_config.start_pos_x,
                          game_inst->app_config.start_pos_y,
                          game_inst->app_config.start_width,
                          game_inst->app_config.start_height))
    {
        return FALSE;
    }

    // Initialize the game.
    if (!app_state.game_inst->initialize(app_state.game_inst))
    {
        AFATAL("Game failed to initialize.");
        return FALSE;
    }

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = TRUE;

    return TRUE;
}

b8 application_run()
{
    AINFO(get_memory_usage_str());

    while (app_state.is_running)
    {
        if (!platform_pump_message(&app_state.platform))
        {
            app_state.is_running = FALSE;
        }

        if (!app_state.is_suspended)
        {
            if (!app_state.game_inst->update(app_state.game_inst, (f32)0))
            {
                AFATAL("Game update failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }

            // Call the game's render routine.
            if (!app_state.game_inst->render(app_state.game_inst, (f32)0))
            {
                AFATAL("Game render failed, shutting down.");
                app_state.is_running = FALSE;
                break;
            }
        }
    }

    app_state.is_running = FALSE;

    shutdown_events();

    platform_shutdown(&app_state.platform);
    shutdown_logging();

    return TRUE;
}
