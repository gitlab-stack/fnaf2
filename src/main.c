/*
 * main.c - Entry point
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Initializes the platform layer, creates the game state,
 * and runs the main game loop.
 */

#include "types.h"
#include "game_defs.h"
#include "platform.h"
#include "game.h"
#include "resource.h"

/* Single global game state - avoids heap allocation */
static Game g_game;

int main(int argc, char *argv[])
{
    FNAF2_UNUSED(argc);
    FNAF2_UNUSED(argv);

    /* Initialize platform */
    if (!platform_init("Five Nights at Freddy's 2", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        return 1;
    }

    /* Initialize game */
    game_init(&g_game);

    /* Load menu resources */
    resource_load_menu();

    /* Run the main game loop */
    game_run(&g_game);

    /* Cleanup */
    game_shutdown(&g_game);
    platform_shutdown();

    return 0;
}
