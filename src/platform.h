/*
 * platform.h - Platform abstraction layer
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * All platform-specific code (rendering, audio, input, timing) is accessed
 * through this interface. To port the game to a new platform, implement
 * all functions declared here. See platform_stub.c for a template.
 */

#ifndef FNAF2_PLATFORM_H
#define FNAF2_PLATFORM_H

#include "types.h"
#include "game_defs.h"

/* ================================================================
 * PLATFORM LIFECYCLE
 * ================================================================ */

/* Initialize the platform (window, renderer, audio, etc.)
 * Returns true on success, false on failure. */
bool platform_init(const char *title, int width, int height);

/* Shut down the platform and release resources */
void platform_shutdown(void);

/* Returns true if the platform wants to quit (window close, etc.) */
bool platform_should_quit(void);

/* ================================================================
 * TIMING
 * ================================================================ */

/* Get current time in milliseconds (monotonic) */
uint32_t platform_get_ticks_ms(void);

/* Sleep for the given number of milliseconds */
void platform_sleep_ms(uint32_t ms);

/* ================================================================
 * INPUT
 * ================================================================ */

/* Poll input events. Call once per frame. */
void platform_poll_input(void);

/* Returns true if button was just pressed this frame */
bool platform_button_pressed(InputButton btn);

/* Returns true if button is currently held down */
bool platform_button_held(InputButton btn);

/* Get cursor/mouse position in virtual screen coordinates */
void platform_get_cursor(int *x, int *y);

/* Returns true if the cursor/mouse is currently pressed */
bool platform_cursor_down(void);

/* Returns true if the cursor was just clicked this frame */
bool platform_cursor_clicked(void);

/* ================================================================
 * RENDERING
 * ================================================================ */

/* Begin a new frame (clear screen, etc.) */
void platform_render_begin(void);

/* End frame and present to screen */
void platform_render_end(void);

/* Draw a sprite/texture at the given position.
 * sprite_id is a ResourceID. x, y are virtual screen coordinates.
 * If the resource isn't loaded, this is a no-op. */
void platform_draw_sprite(int sprite_id, int x, int y);

/* Draw a sprite with additional parameters */
void platform_draw_sprite_ex(int sprite_id, int x, int y,
                              int w, int h, uint8_t alpha,
                              bool flip_h, bool flip_v);

/* Draw a filled rectangle */
void platform_draw_rect(int x, int y, int w, int h,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* Draw text string.
 * size: 0 = small, 1 = medium, 2 = large */
void platform_draw_text(const char *text, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, int size);

/* Draw text centered horizontally at y position */
void platform_draw_text_centered(const char *text, int y,
                                  uint8_t r, uint8_t g, uint8_t b, int size);

/* Set the screen fade/overlay color and alpha (for transitions) */
void platform_set_fade(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/* ================================================================
 * AUDIO
 * ================================================================ */

/* Play a sound effect. Returns a channel/handle ID, or -1 on failure.
 * loop: if true, the sound loops until stopped. */
int platform_play_sound(int sound_id, bool loop);

/* Stop a sound by channel/handle */
void platform_stop_sound(int handle);

/* Stop all currently playing sounds */
void platform_stop_all_sounds(void);

/* Set volume for a sound handle (0-255) */
void platform_set_volume(int handle, int volume);

/* Set master volume (0-255) */
void platform_set_master_volume(int volume);

/* Check if a sound handle is still playing */
bool platform_is_sound_playing(int handle);

/* ================================================================
 * RESOURCES
 * ================================================================ */

/* Load a single resource by ID.
 * The platform maps resource IDs to actual file paths.
 * Returns true on success. */
bool platform_load_resource(int resource_id);

/* Unload a single resource */
void platform_unload_resource(int resource_id);

/* Load all resources needed for a given game state.
 * Implementations should load appropriate assets for the state. */
void platform_load_resources_for_state(GameStateID state);

/* Unload resources for a state */
void platform_unload_resources_for_state(GameStateID state);

/* Check if a resource is loaded */
bool platform_resource_loaded(int resource_id);

/* ================================================================
 * FILE I/O (for save data)
 * ================================================================ */

/* Save game progress (night reached, stars earned, etc.)
 * Returns true on success. */
bool platform_save_data(const void *data, size_t size);

/* Load game progress.
 * Returns true on success (data was loaded). */
bool platform_load_data(void *data, size_t size);

#endif /* FNAF2_PLATFORM_H */
