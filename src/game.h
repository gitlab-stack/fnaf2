/*
 * game.h - Main game state and logic
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Central game state structure and main update/render loop.
 */

#ifndef FNAF2_GAME_H
#define FNAF2_GAME_H

#include "types.h"
#include "game_defs.h"
#include "animatronic.h"
#include "camera.h"
#include "music_box.h"
#include "office.h"
#include "night.h"
#include "menu.h"

/* ================================================================
 * HALLUCINATION STATE
 * ================================================================ */
typedef struct {
    bool        shadow_bonnie_active;
    uint32_t    shadow_bonnie_timer;

    bool        shadow_freddy_active;

    bool        jj_active;
    bool        endoskeleton_active;
    int         endoskeleton_location;  /* Which cam shows endo */

    bool        paper_pal_active;
    int         paper_pal_ai;

    /* Random image value (generated on camera lower) */
    int         random_image_value;
} HallucinationState;

/* ================================================================
 * MAIN GAME STATE
 * ================================================================ */
typedef struct {
    /* Core state */
    GameStateID     state;
    RNG             rng;

    /* Night info */
    int             current_night;      /* 1-7 */
    int             current_hour;       /* 0=12AM, 1=1AM, ... 5=5AM */
    uint32_t        night_timer_ms;     /* Total ms elapsed this night */
    uint32_t        hour_timer_ms;      /* ms elapsed this hour */

    /* Subsystems */
    Animatronic     animatronics[ANIM_COUNT];
    CameraSystem    camera;
    MusicBox        music_box;
    Office          office;
    Menu            menu;

    /* Hallucinations */
    HallucinationState hallucinations;

    /* Movement check accumulator */
    uint32_t        movement_check_timer;

    /* Phone call */
    bool            phone_call_active;
    int             phone_call_handle;  /* Audio handle */

    /* Jumpscare */
    AnimatronicID   jumpscare_anim;     /* Which animatronic is jumpscaring */
    uint32_t        jumpscare_timer;

    /* 6 AM celebration */
    uint32_t        sixam_timer;

    /* Game over */
    uint32_t        gameover_timer;

    /* Night start screen */
    uint32_t        night_start_timer;

    /* Static effect */
    uint32_t        static_timer;
    uint8_t         static_intensity;

    /* Ambient audio handles */
    int             fan_handle;
    int             ambience_handle;

    /* Save data */
    SaveData        save;

    /* Debug */
    bool            debug_mode;
    bool            debug_show_ai;
} Game;

/* ================================================================
 * FUNCTION DECLARATIONS
 * ================================================================ */

/* Initialize the entire game */
void game_init(Game *game);

/* Main update function - call with delta time in ms */
void game_update(Game *game, uint32_t delta_ms);

/* Main render function - draws current state */
void game_render(const Game *game);

/* Start a specific night */
void game_start_night(Game *game, int night);

/* Handle death/jumpscare */
void game_trigger_jumpscare(Game *game, AnimatronicID anim);

/* Advance to 6 AM */
void game_trigger_6am(Game *game);

/* Return to menu */
void game_return_to_menu(Game *game);

/* Run the main game loop (called from main) */
void game_run(Game *game);

/* Clean up game resources */
void game_shutdown(Game *game);

#endif /* FNAF2_GAME_H */
