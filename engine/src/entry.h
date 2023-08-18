#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "core/amemory.h"
#include "game_types.h"

// Externally-defined function to create a game.
extern b8 create_game(game* out_game);

/**
 * The main entry point of the application.
*/
int main(void)
{
    initialize_memory();

    // Request the game instance from the application.
    game game_inst;
    if(!create_game(&game_inst))
    {
        AFATAL("Could not create game!");
        return -1;
    }

    // Ensure that function pointers exist.
    if(!game_inst.initialize || !game_inst.render || !game_inst.update || !game_inst.on_resize)
    {
        AFATAL("The game's function pointers must be assigned.");
        return -2;
    }

    // Initialization.
    if(!application_create(&game_inst))
    {
        AINFO("Application failed to create!");
        return 1;
    }

    // Begin the game loop.
    if(!application_run())
    {
        AINFO("Application did not shutdown gracefully.");
        return 2;
    }

    shutdown_memory();

    return 0;
}
