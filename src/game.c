/*
 * game.c - Main game state and logic implementation
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * This is the central game module that ties all subsystems together:
 * - State machine (menu, playing, jumpscare, 6AM, game over)
 * - Night progression (hour advancement, AI level updates)
 * - Animatronic AI orchestration
 * - Input handling during gameplay
 * - Rendering orchestration
 * - Hallucination system
 */

#include "game.h"
#include "resource.h"
#include "platform.h"

/* ================================================================
 * FORWARD DECLARATIONS
 * ================================================================ */
static void game_update_playing(Game *game, uint32_t delta_ms);
static void game_update_input(Game *game);
static void game_update_animatronics(Game *game, uint32_t delta_ms);
static void game_update_hallucinations(Game *game, uint32_t delta_ms);
static void game_update_night_timer(Game *game, uint32_t delta_ms);
static void game_render_office(const Game *game);
static void game_render_camera(const Game *game);
static void game_render_mask(const Game *game);
static void game_render_hud(const Game *game);
static void game_render_static(const Game *game, uint8_t intensity);
static const char *hour_to_string(int hour);
static void int_to_str(int value, char *buf, int bufsize);

/* ================================================================
 * INITIALIZATION
 * ================================================================ */

void game_init(Game *game)
{
    int i;

    FNAF2_MEMSET(game, 0, sizeof(Game));
    game->state = GAME_STATE_MENU;
    rng_seed(&game->rng, 0x46724564); /* "Fred" in hex-ish */

    for (i = 0; i < ANIM_COUNT; i++) {
        animatronic_init(&game->animatronics[i], (AnimatronicID)i);
    }

    camera_init(&game->camera);
    music_box_init(&game->music_box);
    office_init(&game->office);
    menu_init(&game->menu);

    save_data_init(&game->save);
    menu_load_save(&game->menu);

    game->fan_handle = -1;
    game->ambience_handle = -1;
    game->phone_call_handle = -1;
    game->jumpscare_anim = ANIM_COUNT;

    game->debug_mode = false;
    game->debug_show_ai = false;
}

/* ================================================================
 * START NIGHT
 * ================================================================ */

void game_start_night(Game *game, int night)
{
    int i;
    int drain_rate;

    game->current_night = FNAF2_CLAMP(night, 1, MAX_NIGHTS);
    game->current_hour = 0;
    game->night_timer_ms = 0;
    game->hour_timer_ms = 0;
    game->movement_check_timer = 0;

    /* Reset all animatronics */
    for (i = 0; i < ANIM_COUNT; i++) {
        animatronic_reset(&game->animatronics[i]);
    }

    /* Set initial AI levels (hour 0 = 12 AM) */
    for (i = 0; i < ANIM_COUNT; i++) {
        int ai = night_get_ai_level(game->current_night, (AnimatronicID)i, 0);
        animatronic_set_ai(&game->animatronics[i], ai);
    }

    /* Reset camera */
    camera_reset(&game->camera);

    /* Reset music box with night-specific drain rate */
    drain_rate = night_get_music_box_drain(game->current_night);
    music_box_reset(&game->music_box, drain_rate);

    /* Reset office */
    office_reset(&game->office);

    /* Reset hallucinations */
    FNAF2_MEMSET(&game->hallucinations, 0, sizeof(HallucinationState));

    /* Reset audio */
    platform_stop_all_sounds();
    game->fan_handle = platform_play_sound(SND_FAN_LOOP, true);
    game->ambience_handle = platform_play_sound(SND_AMBIENCE_1, true);

    /* Phone call on nights 1-6 */
    game->phone_call_active = false;
    game->phone_call_handle = -1;
    if (game->current_night <= 6) {
        game->phone_call_active = true;
        game->phone_call_handle = platform_play_sound(
            SND_PHONE_CALL_BASE + (game->current_night - 1), false);
    }

    /* Show night start screen */
    game->state = GAME_STATE_NIGHT_START;
    game->night_start_timer = 0;

    /* Load gameplay resources */
    resource_load_gameplay();
}

/* ================================================================
 * MAIN UPDATE
 * ================================================================ */

void game_update(Game *game, uint32_t delta_ms)
{
    switch (game->state) {
        case GAME_STATE_MENU: {
            int result = menu_update(&game->menu, delta_ms);
            if (result > 0) {
                game_start_night(game, result);
            } else if (result < 0) {
                game->state = GAME_STATE_QUIT;
            }
            break;
        }

        case GAME_STATE_NIGHT_START:
            game->night_start_timer += delta_ms;
            if (game->night_start_timer >= 3000) {
                game->state = GAME_STATE_PLAYING;
            }
            /* Allow skipping */
            if (platform_button_pressed(BTN_ACCEPT) ||
                platform_cursor_clicked()) {
                game->state = GAME_STATE_PLAYING;
            }
            break;

        case GAME_STATE_PLAYING:
            game_update_playing(game, delta_ms);
            break;

        case GAME_STATE_JUMPSCARE:
            game->jumpscare_timer += delta_ms;
            if (game->jumpscare_timer >= JUMPSCARE_DURATION_MS) {
                game->state = GAME_STATE_STATIC;
                game->static_timer = 0;
                platform_stop_all_sounds();
            }
            break;

        case GAME_STATE_STATIC:
            game->static_timer += delta_ms;
            if (game->static_timer >= 2000) {
                game->state = GAME_STATE_GAME_OVER;
                game->gameover_timer = 0;
                platform_stop_all_sounds();
            }
            break;

        case GAME_STATE_GAME_OVER:
            game->gameover_timer += delta_ms;
            if (game->gameover_timer >= 3000 ||
                platform_button_pressed(BTN_ACCEPT) ||
                platform_cursor_clicked()) {
                game_return_to_menu(game);
            }
            break;

        case GAME_STATE_6AM:
            game->sixam_timer += delta_ms;
            if (game->sixam_timer >= 5000 ||
                platform_button_pressed(BTN_ACCEPT) ||
                platform_cursor_clicked()) {
                /* Check for paycheck or go to menu */
                if (game->current_night == 5) {
                    game->state = GAME_STATE_PAYCHECK;
                    game->gameover_timer = 0;
                } else {
                    menu_complete_night(&game->menu, game->current_night);
                    game_return_to_menu(game);
                }
            }
            break;

        case GAME_STATE_PAYCHECK:
            game->gameover_timer += delta_ms;
            if (game->gameover_timer >= 5000 ||
                platform_button_pressed(BTN_ACCEPT) ||
                platform_cursor_clicked()) {
                menu_complete_night(&game->menu, game->current_night);
                game_return_to_menu(game);
            }
            break;

        case GAME_STATE_QUIT:
            /* Handled by main loop */
            break;

        default:
            break;
    }
}

/* ================================================================
 * GAMEPLAY UPDATE
 * ================================================================ */

static void game_update_playing(Game *game, uint32_t delta_ms)
{
    /* Update night timer */
    game_update_night_timer(game, delta_ms);

    /* Check for 6 AM */
    if (game->night_timer_ms >= NIGHT_DURATION_MS) {
        game_trigger_6am(game);
        return;
    }

    /* Handle player input */
    game_update_input(game);

    /* Update office (flashlight, mask, scrolling) */
    office_update(&game->office, delta_ms);

    /* Update camera system */
    camera_update(&game->camera, delta_ms);

    /* Update music box */
    {
        bool winding = camera_on_prize_corner(&game->camera) &&
                       (platform_button_held(BTN_WIND_BOX) ||
                        platform_cursor_down());
        if (winding) {
            music_box_start_wind(&game->music_box);
        } else {
            music_box_stop_wind(&game->music_box);
        }

        if (music_box_update(&game->music_box, delta_ms)) {
            /* Music box ran out - Marionette attacks! */
            game_trigger_jumpscare(game, ANIM_MARIONETTE);
            return;
        }
    }

    /* Update all animatronics */
    game_update_animatronics(game, delta_ms);

    /* Update hallucinations */
    game_update_hallucinations(game, delta_ms);

    /* Check if phone call ended */
    if (game->phone_call_active && game->phone_call_handle >= 0) {
        if (!platform_is_sound_playing(game->phone_call_handle)) {
            game->phone_call_active = false;
            game->phone_call_handle = -1;
        }
    }
}

/* ================================================================
 * NIGHT TIMER
 * ================================================================ */

static void game_update_night_timer(Game *game, uint32_t delta_ms)
{
    int new_hour;
    int i;

    game->night_timer_ms += delta_ms;
    game->hour_timer_ms += delta_ms;

    /* Check for hour advancement */
    new_hour = (int)(game->night_timer_ms / HOUR_DURATION_MS);
    if (new_hour > 5) new_hour = 5;

    if (new_hour != game->current_hour) {
        game->current_hour = new_hour;
        game->hour_timer_ms = 0;

        /* Update AI levels for new hour */
        for (i = 0; i < ANIM_COUNT; i++) {
            int ai = night_get_ai_level(game->current_night,
                                        (AnimatronicID)i,
                                        game->current_hour);
            animatronic_set_ai(&game->animatronics[i], ai);
        }

        /* Play clock chime */
        platform_play_sound(SND_CLOCK_CHIME, false);
    }
}

/* ================================================================
 * INPUT HANDLING
 * ================================================================ */

static void game_update_input(Game *game)
{
    /* Camera toggle */
    if (platform_button_pressed(BTN_CAMERA_TOGGLE)) {
        if (game->camera.is_open) {
            camera_close(&game->camera);
            game->office.current_view = game->office.mask_on ?
                                        VIEW_MASK : VIEW_OFFICE;
        } else if (!game->office.mask_on) {
            camera_open(&game->camera);
            game->office.current_view = VIEW_CAMERA;
            /* Turn off flashlight when opening camera */
            game->office.flashlight_on = false;
        }
    }

    /* Only handle office inputs when camera is down */
    if (!game->camera.is_open) {
        /* Mask toggle */
        if (platform_button_pressed(BTN_MASK_TOGGLE)) {
            office_toggle_mask(&game->office);
        }

        /* Flashlight */
        if (platform_button_pressed(BTN_FLASHLIGHT)) {
            office_toggle_flashlight(&game->office);
        }

        /* Vent lights (held) */
        office_set_vent_light_left(&game->office,
            platform_button_held(BTN_VENT_LIGHT_LEFT));
        office_set_vent_light_right(&game->office,
            platform_button_held(BTN_VENT_LIGHT_RIGHT));

        /* Office scrolling based on cursor position */
        {
            int mx, my;
            platform_get_cursor(&mx, &my);
            FNAF2_UNUSED(my);

            if (mx < SCREEN_WIDTH / 4) {
                game->office.scroll_target = -200;
            } else if (mx > (SCREEN_WIDTH * 3) / 4) {
                game->office.scroll_target = 200;
            } else {
                game->office.scroll_target = 0;
            }
        }
    } else {
        /* Camera is open - handle camera switching */
        if (platform_button_pressed(BTN_CAM_LEFT)) {
            camera_prev(&game->camera);
        }
        if (platform_button_pressed(BTN_CAM_RIGHT)) {
            camera_next(&game->camera);
        }

        /* Click on camera map to switch cameras */
        if (platform_cursor_clicked()) {
            int mx, my;
            platform_get_cursor(&mx, &my);

            /* Camera map is in the bottom-right area */
            if (my > 550 && mx > 600) {
                /* Map camera buttons layout (approximate) */
                int map_x = (mx - 600) / 80;
                int map_y = (my - 550) / 50;
                int cam_id = map_y * 3 + map_x;
                if (cam_id >= 0 && cam_id < CAM_COUNT) {
                    camera_switch(&game->camera, cam_id);
                }
            }
        }

        /* Wind music box (click on wind button when on prize corner cam) */
        if (camera_on_prize_corner(&game->camera)) {
            /* Wind button handled in the playing update above */
        }
    }
}

/* ================================================================
 * ANIMATRONIC UPDATE
 * ================================================================ */

static void game_update_animatronics(Game *game, uint32_t delta_ms)
{
    int i;
    bool any_in_office;
    AnimatronicID attacker;

    any_in_office = animatronics_any_in_office(game->animatronics, ANIM_COUNT);

    for (i = 0; i < ANIM_COUNT; i++) {
        Animatronic *anim = &game->animatronics[i];

        /* Skip Marionette (handled by music box) */
        if (anim->id == ANIM_MARIONETTE) continue;

        attacker = animatronic_update(
            anim, delta_ms, &game->rng,
            game->camera.is_open,
            game->office.mask_on,
            game->office.flashlight_on,
            game->office.vent_light_left,
            game->office.vent_light_right,
            game->current_night,
            any_in_office
        );

        if (attacker < ANIM_COUNT) {
            /* Special case: Balloon Boy doesn't jumpscare, just disables flashlight */
            if (attacker == ANIM_BALLOON_BOY) {
                office_disable_flashlight(&game->office);
                anim->state = ANIM_STATE_IN_OFFICE;
                /* BB stays in office, laughing */
                platform_play_sound(SND_DEEP_LAUGH, false);
            } else {
                game_trigger_jumpscare(game, attacker);
                return;
            }
        }
    }

    /* Stun animatronics when viewed on camera */
    if (game->camera.is_open) {
        for (i = 0; i < ANIM_COUNT; i++) {
            Animatronic *anim = &game->animatronics[i];
            if (anim->is_active &&
                IS_CAMERA_LOC(anim->position) &&
                (int)anim->position == game->camera.current_cam) {
                /* Viewing an animatronic on camera stuns it briefly */
                if (anim->state == ANIM_STATE_IDLE) {
                    animatronic_stun(anim, 500);
                }
            }
        }
    }
}

/* ================================================================
 * HALLUCINATION UPDATE
 * ================================================================ */

static void game_update_hallucinations(Game *game, uint32_t delta_ms)
{
    HallucinationState *h = &game->hallucinations;
    FNAF2_UNUSED(delta_ms);

    /*
     * Shadow Bonnie (RWQFSFASXC):
     * 1 in 1,000,000 chance when lowering camera.
     * If triggered, crashes ~4 seconds later.
     */
    if (!h->shadow_bonnie_active) {
        /* Check is done when camera is lowered (in input handler) */
        /* Simplified: check each frame while camera transitions */
        if (!game->camera.is_open && game->camera.open_timer > 0) {
            if (rng_range(&game->rng, SHADOW_BONNIE_CHANCE) == 0) {
                h->shadow_bonnie_active = true;
                h->shadow_bonnie_timer = 4000;
            }
        }
    } else {
        if (h->shadow_bonnie_timer > delta_ms) {
            h->shadow_bonnie_timer -= delta_ms;
        } else {
            /* "Crash" - in our implementation, trigger game over */
            game_trigger_jumpscare(game, ANIM_WITHERED_BONNIE);
        }
    }

    /*
     * Shadow Freddy:
     * Random image value 0-1000 generated when camera lowers.
     * If <= 100 and Parts/Service (CAM 08) is empty, Shadow Freddy appears.
     */
    /* This is checked per-frame for simplicity */
    if (!game->camera.is_open && !h->shadow_freddy_active) {
        h->random_image_value = (int)rng_range(&game->rng, 1001);
        if (h->random_image_value <= SHADOW_FREDDY_CHANCE) {
            if (!animatronics_any_at_location(game->animatronics,
                                               ANIM_COUNT,
                                               CAM_PARTS_SERVICE)) {
                h->shadow_freddy_active = true;
            }
        }

        /* JJ: appears if value == 9 */
        if (h->random_image_value == 9) {
            h->jj_active = true;
        }

        /* Endoskeleton: value == 3 (left vent) or <= 100 (prize corner) */
        if (h->random_image_value == 3) {
            h->endoskeleton_active = true;
            h->endoskeleton_location = CAM_LEFT_VENT;
        } else if (h->random_image_value <= 100) {
            /* Only if Marionette is active */
            if (game->animatronics[ANIM_MARIONETTE].is_active) {
                h->endoskeleton_active = true;
                h->endoskeleton_location = CAM_PRIZE_CORNER;
            }
        }
    }

    /* Clear hallucinations when camera goes back up */
    if (game->camera.is_open) {
        h->shadow_freddy_active = false;
        h->jj_active = false;
        h->endoskeleton_active = false;
    }
}

/* ================================================================
 * STATE TRANSITIONS
 * ================================================================ */

void game_trigger_jumpscare(Game *game, AnimatronicID anim)
{
    game->state = GAME_STATE_JUMPSCARE;
    game->jumpscare_anim = anim;
    game->jumpscare_timer = 0;

    /* Close camera/mask */
    game->camera.is_open = false;
    game->office.mask_on = false;
    game->office.current_view = VIEW_OFFICE;

    /* Stop ambient sounds */
    if (game->fan_handle >= 0) {
        platform_stop_sound(game->fan_handle);
        game->fan_handle = -1;
    }
    if (game->ambience_handle >= 0) {
        platform_stop_sound(game->ambience_handle);
        game->ambience_handle = -1;
    }
    if (game->phone_call_handle >= 0) {
        platform_stop_sound(game->phone_call_handle);
        game->phone_call_handle = -1;
    }

    platform_play_sound(SND_JUMPSCARE, false);
}

void game_trigger_6am(Game *game)
{
    game->state = GAME_STATE_6AM;
    game->sixam_timer = 0;

    platform_stop_all_sounds();
    platform_play_sound(SND_CLOCK_CHIME, false);
}

void game_return_to_menu(Game *game)
{
    game->state = GAME_STATE_MENU;
    platform_stop_all_sounds();
    resource_unload_gameplay();
    resource_load_menu();

    game->fan_handle = -1;
    game->ambience_handle = -1;
    game->phone_call_handle = -1;
    game->phone_call_active = false;
}

/* ================================================================
 * RENDERING
 * ================================================================ */

void game_render(const Game *game)
{
    platform_render_begin();

    switch (game->state) {
        case GAME_STATE_MENU:
            menu_render(&game->menu);
            break;

        case GAME_STATE_NIGHT_START: {
            char night_str[16];
            char hour_str[8];

            /* Dark screen with night number and "12 AM" */
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);

            /* "Night X" */
            night_str[0] = 'N'; night_str[1] = 'i'; night_str[2] = 'g';
            night_str[3] = 'h'; night_str[4] = 't'; night_str[5] = ' ';
            night_str[6] = '0' + (char)game->current_night;
            night_str[7] = '\0';
            platform_draw_text_centered(night_str, 350, 255, 255, 255, 2);

            /* "12 AM" */
            hour_str[0] = '1'; hour_str[1] = '2'; hour_str[2] = ' ';
            hour_str[3] = 'A'; hour_str[4] = 'M'; hour_str[5] = '\0';
            platform_draw_text_centered(hour_str, 420, 200, 200, 200, 1);
            break;
        }

        case GAME_STATE_PLAYING:
            if (game->camera.is_open) {
                game_render_camera(game);
            } else if (game->office.mask_on) {
                game_render_mask(game);
            } else {
                game_render_office(game);
            }
            game_render_hud(game);
            break;

        case GAME_STATE_JUMPSCARE:
            /* Draw jumpscare sprite */
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
            platform_draw_sprite(RES_JUMPSCARE_BASE + game->jumpscare_anim,
                                 0, 0);
            /* Add some static */
            game_render_static(game, 100);
            break;

        case GAME_STATE_STATIC:
            game_render_static(game, 255);
            break;

        case GAME_STATE_GAME_OVER:
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
            platform_draw_sprite(RES_GAME_OVER, 0, 0);
            /* "Game Over" text fallback */
            platform_draw_text_centered("Game Over", 384, 200, 0, 0, 2);
            break;

        case GAME_STATE_6AM: {
            char night_str[16];
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
            platform_draw_text_centered("6 AM", 300, 255, 255, 255, 2);

            night_str[0] = 'N'; night_str[1] = 'i'; night_str[2] = 'g';
            night_str[3] = 'h'; night_str[4] = 't'; night_str[5] = ' ';
            night_str[6] = '0' + (char)game->current_night;
            night_str[7] = '\0';
            platform_draw_text_centered(night_str, 380, 200, 200, 200, 1);
            platform_draw_text_centered("Complete!", 440, 200, 200, 200, 1);
            break;
        }

        case GAME_STATE_PAYCHECK:
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
            platform_draw_sprite(RES_PAYCHECK, 0, 0);
            break;

        default:
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 255);
            break;
    }

    /* Debug overlay */
    if (game->debug_mode && game->state == GAME_STATE_PLAYING) {
        int i;
        for (i = 0; i < ANIM_COUNT; i++) {
            const Animatronic *a = &game->animatronics[i];
            char buf[64];
            int len = 0;
            const char *name = animatronic_get_name(i);

            /* Copy name */
            while (*name && len < 30) {
                buf[len++] = *name++;
            }
            buf[len++] = ':';

            /* AI level */
            int_to_str(a->ai_level, buf + len, (int)(sizeof(buf)) - len);
            while (buf[len]) len++;

            buf[len++] = ' ';
            buf[len++] = 'P';
            buf[len++] = ':';
            int_to_str((int)a->position, buf + len, (int)(sizeof(buf)) - len);
            while (buf[len]) len++;

            buf[len] = '\0';
            platform_draw_text(buf, 10, 10 + i * 18, 0, 255, 0, 0);
        }

        /* Music box timer */
        {
            char mb_buf[32];
            int len = 0;
            mb_buf[len++] = 'M';
            mb_buf[len++] = 'B';
            mb_buf[len++] = ':';
            int_to_str(game->music_box.timer, mb_buf + len,
                       (int)sizeof(mb_buf) - len);
            while (mb_buf[len]) len++;
            mb_buf[len] = '\0';
            platform_draw_text(mb_buf, 10, 10 + ANIM_COUNT * 18,
                              255, 255, 0, 0);
        }
    }

    platform_render_end();
}

/* ================================================================
 * RENDER: OFFICE VIEW
 * ================================================================ */

static void game_render_office(const Game *game)
{
    int offset = game->office.scroll_x;
    int i;

    /* Draw office background */
    if (game->office.flashlight_on) {
        platform_draw_sprite(RES_OFFICE_FLASHLIGHT, -offset, 0);
    } else {
        platform_draw_sprite(RES_OFFICE_DARK, -offset, 0);
    }

    /* Vent lights */
    if (game->office.vent_light_left) {
        platform_draw_sprite(RES_OFFICE_LEFT_LIGHT, -offset, 0);
    }
    if (game->office.vent_light_right) {
        platform_draw_sprite(RES_OFFICE_RIGHT_LIGHT, -offset, 0);
    }

    /* Draw animatronics visible in office area */
    for (i = 0; i < ANIM_COUNT; i++) {
        const Animatronic *a = &game->animatronics[i];
        if (!a->is_active) continue;

        /* Animatronic in hallway (visible with flashlight) */
        if (a->position == LOC_HALLWAY && game->office.flashlight_on) {
            platform_draw_sprite(RES_ANIM_HALLWAY_BASE + i,
                                 SCREEN_WIDTH / 2 - 100 - offset, 100);
        }

        /* Animatronic at left vent entrance */
        if (a->position == LOC_LEFT_VENT_NEAR && game->office.vent_light_left) {
            platform_draw_sprite(RES_ANIM_LEFT_VENT_BASE + i,
                                 50 - offset, 200);
        }

        /* Animatronic at right vent entrance */
        if (a->position == LOC_RIGHT_VENT_NEAR && game->office.vent_light_right) {
            platform_draw_sprite(RES_ANIM_RIGHT_VENT_BASE + i,
                                 SCREEN_WIDTH - 200 - offset, 200);
        }

        /* Animatronic in office */
        if (a->position == LOC_OFFICE && a->state == ANIM_STATE_IN_OFFICE) {
            platform_draw_sprite(RES_ANIM_OFFICE_BASE + i,
                                 SCREEN_WIDTH / 2 - 150, 50);
        }
    }

    /* Golden Freddy hallway appearance */
    if (game->animatronics[ANIM_GOLDEN_FREDDY].gf_in_hallway &&
        game->office.flashlight_on) {
        platform_draw_sprite(RES_ANIM_HALLWAY_BASE + ANIM_GOLDEN_FREDDY,
                             SCREEN_WIDTH / 2 - 100 - offset, 100);
    }

    /* Golden Freddy office appearance */
    if (game->animatronics[ANIM_GOLDEN_FREDDY].gf_in_office) {
        platform_draw_sprite(RES_ANIM_OFFICE_BASE + ANIM_GOLDEN_FREDDY,
                             SCREEN_WIDTH / 2 - 150, 50);
    }

    /* Hallucinations */
    if (game->hallucinations.shadow_freddy_active) {
        /* Shadow Freddy appears slumped in office */
        platform_draw_rect(350, 400, 200, 300, 40, 0, 80, 180);
    }

    if (game->hallucinations.jj_active) {
        /* JJ appears under desk */
        platform_draw_rect(400, 600, 150, 100, 80, 40, 120, 200);
    }

    /* Light static flicker effect */
    if (game->office.flashlight_on) {
        uint8_t flicker = (uint8_t)(rng_range((RNG *)&game->rng, 30));
        game_render_static(game, flicker);
    }
}

/* ================================================================
 * RENDER: CAMERA VIEW
 * ================================================================ */

static void game_render_camera(const Game *game)
{
    int cam = game->camera.current_cam;
    int i;

    /* Camera feed background */
    if (game->camera.static_active) {
        /* Show static while switching */
        platform_draw_sprite(RES_CAMERA_STATIC, 0, 0);
    } else {
        /* Show camera feed */
        platform_draw_sprite(RES_CAM_FEED_BASE + cam, 0, 0);

        /* Draw animatronics visible on this camera */
        for (i = 0; i < ANIM_COUNT; i++) {
            const Animatronic *a = &game->animatronics[i];
            if (a->is_active && IS_CAMERA_LOC(a->position) &&
                (int)a->position == cam) {
                platform_draw_sprite(
                    RES_ANIM_CAM_BASE + (i * CAM_COUNT + cam),
                    SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 150);
            }
        }

        /* Hallucination on camera */
        if (game->hallucinations.endoskeleton_active &&
            game->hallucinations.endoskeleton_location == cam) {
            /* Draw endoskeleton */
            platform_draw_rect(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 - 100,
                              100, 200, 180, 180, 180, 200);
        }
    }

    /* Camera border/overlay */
    platform_draw_sprite(RES_CAMERA_BORDER, 0, 0);

    /* Camera name */
    if (cam >= 0 && cam < CAM_COUNT) {
        platform_draw_text(CAMERA_NAMES[cam], 30, 30, 255, 255, 255, 1);
        platform_draw_text(CAMERA_AREA_NAMES[cam], 30, 60, 200, 200, 200, 0);
    }

    /* Recording indicator */
    platform_draw_sprite(RES_CAMERA_RECORDING, SCREEN_WIDTH - 100, 30);

    /* Camera map */
    platform_draw_sprite(RES_CAMERA_MAP, 600, 550);

    /* Music box UI on Prize Corner */
    if (cam == CAM_PRIZE_CORNER) {
        int bar_width;

        platform_draw_sprite(RES_UI_WIND_BUTTON,
                             SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 120);

        /* Music box timer bar */
        bar_width = FIXED_TO_INT(FIXED_MUL(
            music_box_get_percent(&game->music_box), INT_TO_FIXED(200)));
        platform_draw_rect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 60,
                          200, 20, 60, 60, 60, 200);
        platform_draw_rect(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 60,
                          bar_width, 20, 0, 200, 0, 255);

        /* Warning flash */
        if (music_box_is_warning(&game->music_box)) {
            uint8_t flash = (uint8_t)((game->night_timer_ms / 200) % 2 ? 100 : 0);
            platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                              255, 0, 0, flash);
        }
    }

    /* Scan lines / static overlay */
    game_render_static(game, 30);
}

/* ================================================================
 * RENDER: MASK VIEW
 * ================================================================ */

static void game_render_mask(const Game *game)
{
    int i;

    /* Office with mask overlay */
    platform_draw_sprite(RES_OFFICE_DARK, 0, 0);
    platform_draw_sprite(RES_OFFICE_MASK, 0, 0);

    /* Animatronics visible while wearing mask */
    for (i = 0; i < ANIM_COUNT; i++) {
        const Animatronic *a = &game->animatronics[i];
        if (!a->is_active) continue;

        /* Show animatronics that are in the office/near vents */
        if (a->state == ANIM_STATE_IN_OFFICE ||
            a->state == ANIM_STATE_RETREATING) {
            platform_draw_sprite_ex(RES_ANIM_OFFICE_BASE + i,
                                    SCREEN_WIDTH / 2 - 150, 50,
                                    0, 0, 180, false, false);
        }

        if (a->position == LOC_LEFT_VENT_NEAR) {
            platform_draw_sprite_ex(RES_ANIM_LEFT_VENT_BASE + i,
                                    50, 200, 0, 0, 180, false, false);
        }
        if (a->position == LOC_RIGHT_VENT_NEAR) {
            platform_draw_sprite_ex(RES_ANIM_RIGHT_VENT_BASE + i,
                                    SCREEN_WIDTH - 200, 200,
                                    0, 0, 180, false, false);
        }
    }

    /* Breathing effect - slight darkening */
    platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 60);
}

/* ================================================================
 * RENDER: HUD
 * ================================================================ */

static void game_render_hud(const Game *game)
{
    char buf[32];
    int len;
    const char *hour_str;

    /* Time display - top right */
    hour_str = hour_to_string(game->current_hour);
    platform_draw_text(hour_str, SCREEN_WIDTH - 120, 20, 255, 255, 255, 1);

    /* Night display */
    buf[0] = 'N'; buf[1] = 'i'; buf[2] = 'g'; buf[3] = 'h';
    buf[4] = 't'; buf[5] = ' ';
    buf[6] = '0' + (char)game->current_night;
    buf[7] = '\0';
    platform_draw_text(buf, SCREEN_WIDTH - 120, 50, 200, 200, 200, 0);

    /* Music box indicator (bottom, always visible) */
    {
        int bar_w = FIXED_TO_INT(FIXED_MUL(
            music_box_get_percent(&game->music_box), INT_TO_FIXED(150)));
        uint8_t bar_r = 0, bar_g = 200, bar_b = 0;

        if (music_box_is_warning(&game->music_box)) {
            bar_r = 255; bar_g = 50; bar_b = 0;
        }

        /* Background */
        platform_draw_rect(10, SCREEN_HEIGHT - 30, 150, 15, 40, 40, 40, 200);
        /* Fill */
        platform_draw_rect(10, SCREEN_HEIGHT - 30, bar_w, 15,
                          bar_r, bar_g, bar_b, 255);
        /* Label */
        platform_draw_text("Music Box", 10, SCREEN_HEIGHT - 48,
                          180, 180, 180, 0);
    }

    /* Flashlight battery indicator */
    if (!game->camera.is_open && !game->office.mask_on) {
        int batt_w;

        len = 0;
        buf[len++] = 'F';
        buf[len++] = 'L';
        buf[len] = '\0';

        batt_w = (game->office.flashlight_battery * 60) / FLASHLIGHT_MAX_BATTERY;
        platform_draw_rect(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30,
                          60, 15, 40, 40, 40, 200);
        platform_draw_rect(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 30,
                          batt_w, 15, 255, 255, 100, 255);
        platform_draw_text(buf, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 48,
                          180, 180, 180, 0);

        if (game->office.flashlight_disabled) {
            platform_draw_text("DISABLED", SCREEN_WIDTH - 80,
                              SCREEN_HEIGHT - 48, 255, 0, 0, 0);
        }
    }

    /* Camera toggle button hint */
    if (!game->camera.is_open && !game->office.mask_on) {
        platform_draw_sprite(RES_UI_CAMERA_TOGGLE,
                             SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 60);
    }
}

/* ================================================================
 * RENDER: STATIC EFFECT
 * ================================================================ */

static void game_render_static(const Game *game, uint8_t intensity)
{
    FNAF2_UNUSED(game);
    if (intensity == 0) return;

    platform_draw_sprite_ex(RES_STATIC_OVERLAY, 0, 0,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            intensity, false, false);
}

/* ================================================================
 * MAIN GAME LOOP
 * ================================================================ */

void game_run(Game *game)
{
    uint32_t last_time;
    uint32_t accumulator = 0;

    last_time = platform_get_ticks_ms();

    while (game->state != GAME_STATE_QUIT && !platform_should_quit()) {
        uint32_t current_time = platform_get_ticks_ms();
        uint32_t frame_time = current_time - last_time;
        last_time = current_time;

        /* Cap frame time to prevent spiral of death */
        if (frame_time > 250) {
            frame_time = 250;
        }

        /* Poll input */
        platform_poll_input();

        /* Fixed timestep update */
        accumulator += frame_time;
        while (accumulator >= TICK_RATE_MS) {
            game_update(game, TICK_RATE_MS);
            accumulator -= TICK_RATE_MS;
        }

        /* Render */
        game_render(game);

        /* Seed RNG with timing entropy */
        game->rng.state ^= current_time;

        /* Frame rate limiting */
        {
            uint32_t elapsed = platform_get_ticks_ms() - current_time;
            if (elapsed < 16) { /* ~60 FPS cap */
                platform_sleep_ms(16 - elapsed);
            }
        }
    }
}

/* ================================================================
 * SHUTDOWN
 * ================================================================ */

void game_shutdown(Game *game)
{
    platform_stop_all_sounds();
    resource_unload_gameplay();
    resource_unload_menu();
    FNAF2_UNUSED(game);
}

/* ================================================================
 * HELPER FUNCTIONS
 * ================================================================ */

static const char *hour_to_string(int hour)
{
    switch (hour) {
        case 0: return "12 AM";
        case 1: return "1 AM";
        case 2: return "2 AM";
        case 3: return "3 AM";
        case 4: return "4 AM";
        case 5: return "5 AM";
        default: return "?? AM";
    }
}

static void int_to_str(int value, char *buf, int bufsize)
{
    int i = 0;
    int start, end;
    bool negative = false;

    if (bufsize <= 0) return;

    if (value < 0) {
        negative = true;
        value = -value;
    }

    /* Generate digits in reverse */
    do {
        if (i >= bufsize - 1) break;
        buf[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (negative && i < bufsize - 1) {
        buf[i++] = '-';
    }

    buf[i] = '\0';

    /* Reverse the string */
    start = 0;
    end = i - 1;
    while (start < end) {
        char tmp = buf[start];
        buf[start] = buf[end];
        buf[end] = tmp;
        start++;
        end--;
    }
}
