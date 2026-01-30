/*
 * platform_stub.c - Stub/template platform backend for porting
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * ====================================================================
 * HOW TO PORT TO A NEW PLATFORM
 * ====================================================================
 *
 * 1. Copy this file and rename it (e.g., platform_ps1.c, platform_xbox360.c)
 *
 * 2. Define your platform macro (e.g., FNAF2_PLATFORM_PS1) and guard
 *    all code with #ifdef FNAF2_PLATFORM_PS1 ... #endif
 *
 * 3. Implement ALL functions below using your platform's APIs:
 *    - Rendering: Use your GPU/display API to draw sprites and rectangles
 *    - Audio: Use your sound hardware API to play WAV/PCM audio
 *    - Input: Map your controller/input device to the InputButton enum
 *    - Timing: Use your platform's timer/tick counter
 *    - File I/O: Use your memory card/storage API for save data
 *
 * 4. Resource loading:
 *    - Assets are referenced by ResourceID (integer enum)
 *    - resource_get_path() returns a file path string
 *    - Convert asset formats as needed (PNG -> your texture format, etc.)
 *    - For consoles, you may want to pack assets into a single archive
 *
 * 5. Build:
 *    - Compile all src/ .c files plus your platform file
 *    - Define your platform macro (e.g., -DFNAF2_PLATFORM_PS1)
 *    - Do NOT compile platform_sdl2.c or any other platform file
 *
 * 6. Platform-specific considerations:
 *    - PS1: Use fixed-point math (define FNAF2_NO_FLOAT). 320x240 resolution.
 *      Map rendering to GTE/GPU primitives. Use SPU for audio.
 *    - Xbox 360: Use XDK APIs. 720p/1080p resolution.
 *      Map rendering to D3D9. Use XAudio2 for audio.
 *    - GBA: Very limited. 240x160 resolution, tile-based rendering.
 *      Define FNAF2_NO_STDLIB and FNAF2_NO_STDINT.
 *    - Switch: Use libnx or official SDK. Similar to SDL2 approach.
 *
 * ====================================================================
 */

#ifdef FNAF2_PLATFORM_STUB

#include "../platform.h"
#include <stdio.h>

/* ---- Internal state ---- */
static bool g_running = true;
static uint32_t g_tick_counter = 0;

/* ================================================================
 * PLATFORM LIFECYCLE
 * ================================================================ */

bool platform_init(const char *title, int width, int height)
{
    printf("[STUB] platform_init: %s (%dx%d)\n", title, width, height);
    /* TODO: Initialize your display, audio hardware, input devices */
    return true;
}

void platform_shutdown(void)
{
    printf("[STUB] platform_shutdown\n");
    /* TODO: Release hardware resources */
}

bool platform_should_quit(void)
{
    return !g_running;
}

/* ================================================================
 * TIMING
 * ================================================================ */

uint32_t platform_get_ticks_ms(void)
{
    /* TODO: Return milliseconds from a monotonic timer */
    return g_tick_counter;
}

void platform_sleep_ms(uint32_t ms)
{
    /* TODO: Sleep/yield for the given duration */
    g_tick_counter += ms;
    (void)ms;
}

/* ================================================================
 * INPUT
 * ================================================================ */

void platform_poll_input(void)
{
    /* TODO: Read controller/keyboard state and update button arrays */
    g_tick_counter += 16; /* Simulate ~60fps */
}

bool platform_button_pressed(InputButton btn)
{
    (void)btn;
    /* TODO: Return true if button was JUST pressed this frame */
    return false;
}

bool platform_button_held(InputButton btn)
{
    (void)btn;
    /* TODO: Return true if button is currently held */
    return false;
}

void platform_get_cursor(int *x, int *y)
{
    /* TODO: Return pointer/cursor position.
     * For controller-only platforms, simulate a cursor with the d-pad. */
    if (x) *x = SCREEN_WIDTH / 2;
    if (y) *y = SCREEN_HEIGHT / 2;
}

bool platform_cursor_down(void)
{
    /* TODO: Return true if primary action button is held */
    return false;
}

bool platform_cursor_clicked(void)
{
    /* TODO: Return true if primary action button was just pressed */
    return false;
}

/* ================================================================
 * RENDERING
 * ================================================================ */

void platform_render_begin(void)
{
    /* TODO: Begin frame (clear framebuffer, set up rendering state) */
}

void platform_render_end(void)
{
    /* TODO: End frame (swap buffers, vsync wait) */
}

void platform_draw_sprite(int sprite_id, int x, int y)
{
    (void)sprite_id;
    (void)x;
    (void)y;
    /* TODO: Draw texture/sprite at (x, y) in virtual coordinates.
     *
     * Implementation notes:
     * - Map sprite_id to your loaded texture/image
     * - Scale from virtual coords (1024x768) to your actual resolution
     * - Handle the case where sprite_id is not loaded (no-op or placeholder)
     *
     * For tile-based hardware (GBA, SNES):
     *   Pre-convert sprites to tile format and use hardware sprite/BG layers.
     *
     * For 3D hardware (PS1, PS2, Xbox):
     *   Draw a textured quad at the screen position.
     */
}

void platform_draw_sprite_ex(int sprite_id, int x, int y,
                              int w, int h, uint8_t alpha,
                              bool flip_h, bool flip_v)
{
    (void)sprite_id; (void)x; (void)y;
    (void)w; (void)h; (void)alpha;
    (void)flip_h; (void)flip_v;
    /* TODO: Extended sprite drawing with size, alpha, and flip support.
     *
     * If your platform doesn't support alpha blending:
     *   - Use stipple patterns or skip semi-transparent draws.
     *
     * If your platform doesn't support arbitrary scaling:
     *   - Pre-scale assets at load time or use nearest-neighbor in software.
     */
}

void platform_draw_rect(int x, int y, int w, int h,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    (void)x; (void)y; (void)w; (void)h;
    (void)r; (void)g; (void)b; (void)a;
    /* TODO: Draw a filled rectangle with the given color.
     *
     * This is used for:
     * - Screen fades and overlays
     * - UI bars (music box timer, flashlight battery)
     * - Debug visualization
     * - Background fills
     */
}

void platform_draw_text(const char *text, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, int size)
{
    (void)text; (void)x; (void)y;
    (void)r; (void)g; (void)b; (void)size;
    /* TODO: Draw text string at (x, y).
     *
     * size: 0=small (~14px), 1=medium (~24px), 2=large (~48px)
     *
     * Options for implementation:
     * - Bitmap font (most portable - pre-rendered character tiles)
     * - System font API (if available)
     * - TTF rendering library
     */
}

void platform_draw_text_centered(const char *text, int y,
                                  uint8_t r, uint8_t g, uint8_t b, int size)
{
    /* Calculate text width and center it */
    int char_w = (size == 2) ? 24 : (size == 1) ? 14 : 8;
    int len = 0;
    while (text[len]) len++;
    platform_draw_text(text, (SCREEN_WIDTH - len * char_w) / 2, y,
                      r, g, b, size);
}

void platform_set_fade(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    platform_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, r, g, b, a);
}

/* ================================================================
 * AUDIO
 * ================================================================ */

int platform_play_sound(int sound_id, bool loop)
{
    (void)sound_id;
    (void)loop;
    /* TODO: Play a sound effect.
     *
     * Return a handle/channel ID that can be used to stop or query the sound.
     * Return -1 if the sound can't be played.
     *
     * If loop is true, the sound should repeat until explicitly stopped.
     *
     * Audio format notes:
     * - The game expects WAV/PCM audio files
     * - Convert to your platform's format at build time or load time
     * - PS1: Convert to VAG format, use SPU
     * - GBA: Convert to 8-bit PCM, use DMA sound channels
     */
    return -1;
}

void platform_stop_sound(int handle)
{
    (void)handle;
    /* TODO: Stop a sound by its channel/handle */
}

void platform_stop_all_sounds(void)
{
    /* TODO: Stop all currently playing sounds */
}

void platform_set_volume(int handle, int volume)
{
    (void)handle;
    (void)volume;
    /* TODO: Set volume for a channel (0-255) */
}

void platform_set_master_volume(int volume)
{
    (void)volume;
    /* TODO: Set master volume (0-255) */
}

bool platform_is_sound_playing(int handle)
{
    (void)handle;
    /* TODO: Check if a sound channel is still playing */
    return false;
}

/* ================================================================
 * RESOURCES
 * ================================================================ */

bool platform_load_resource(int resource_id)
{
    (void)resource_id;
    /* TODO: Load a game resource (texture, sound, etc.)
     *
     * Use resource_get_path(resource_id) to get the asset file path.
     * Convert the path if your platform uses a different file system.
     *
     * For cartridge/disc-based platforms:
     *   Resources may be packed into a single archive file.
     *   Use resource_id as an index into the archive.
     */
    return false;
}

void platform_unload_resource(int resource_id)
{
    (void)resource_id;
    /* TODO: Free memory for a resource */
}

void platform_load_resources_for_state(GameStateID state)
{
    (void)state;
    /* TODO: Batch-load resources for a game state.
     * This is called by resource.c which handles the specific IDs. */
}

void platform_unload_resources_for_state(GameStateID state)
{
    (void)state;
}

bool platform_resource_loaded(int resource_id)
{
    (void)resource_id;
    return false;
}

/* ================================================================
 * FILE I/O
 * ================================================================ */

bool platform_save_data(const void *data, size_t size)
{
    (void)data;
    (void)size;
    /* TODO: Save data to persistent storage.
     *
     * Platform examples:
     * - PC: Write to a file
     * - PS1: Write to memory card (requires BIOS calls)
     * - Xbox: Write to hard drive or memory unit
     * - GBA: Write to SRAM/Flash on cartridge
     */
    return false;
}

bool platform_load_data(void *data, size_t size)
{
    (void)data;
    (void)size;
    /* TODO: Load data from persistent storage */
    return false;
}

#endif /* FNAF2_PLATFORM_STUB */
