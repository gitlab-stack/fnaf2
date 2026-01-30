/*
 * platform_sdl2.c - SDL2 platform backend
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Full platform implementation using SDL2 + SDL2_image + SDL2_mixer.
 * This is the reference implementation for PC (Linux, Windows, macOS).
 *
 * Compile with:
 *   -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
 *
 * If SDL2 is not available, use platform_stub.c as a starting point
 * for your platform port.
 */

#ifdef FNAF2_PLATFORM_SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "../platform.h"

/* ================================================================
 * INTERNAL STATE
 * ================================================================ */

#define MAX_TEXTURES    512
#define MAX_SOUNDS      64
#define MAX_CHANNELS    32
#define SAVE_FILENAME   "fnaf2_save.dat"

typedef struct {
    SDL_Window      *window;
    SDL_Renderer    *renderer;
    int              window_w;
    int              window_h;
    bool             quit_requested;

    /* Textures */
    SDL_Texture     *textures[MAX_TEXTURES];
    int              tex_w[MAX_TEXTURES];
    int              tex_h[MAX_TEXTURES];

    /* Audio */
    Mix_Chunk       *sounds[MAX_SOUNDS];
    Mix_Music       *music;

    /* Font */
    TTF_Font        *font_small;
    TTF_Font        *font_medium;
    TTF_Font        *font_large;

    /* Input */
    bool             keys_pressed[BTN_COUNT];
    bool             keys_held[BTN_COUNT];
    int              mouse_x;
    int              mouse_y;
    bool             mouse_down;
    bool             mouse_clicked;

    /* Scale factors for virtual -> actual coordinates */
    float            scale_x;
    float            scale_y;
    int              offset_x;
    int              offset_y;
} PlatformSDL2;

static PlatformSDL2 g_plat;

/* ================================================================
 * KEY MAPPING
 * ================================================================ */

static InputButton sdl_key_to_button(SDL_Keycode key)
{
    switch (key) {
        case SDLK_SPACE:    return BTN_CAMERA_TOGGLE;
        case SDLK_TAB:      return BTN_CAMERA_TOGGLE;
        case SDLK_f:        return BTN_MASK_TOGGLE;
        case SDLK_LCTRL:    return BTN_FLASHLIGHT;
        case SDLK_RCTRL:    return BTN_FLASHLIGHT;
        case SDLK_q:        return BTN_VENT_LIGHT_LEFT;
        case SDLK_e:        return BTN_VENT_LIGHT_RIGHT;
        case SDLK_a:        return BTN_CAM_LEFT;
        case SDLK_LEFT:     return BTN_CAM_LEFT;
        case SDLK_d:        return BTN_CAM_RIGHT;
        case SDLK_RIGHT:    return BTN_CAM_RIGHT;
        case SDLK_w:        return BTN_WIND_BOX;
        case SDLK_UP:       return BTN_WIND_BOX;
        case SDLK_RETURN:   return BTN_ACCEPT;
        case SDLK_ESCAPE:   return BTN_BACK;
        case SDLK_p:        return BTN_PAUSE;
        default:            return BTN_COUNT;
    }
}

/* ================================================================
 * COORDINATE TRANSFORMATION
 * ================================================================ */

static void virtual_to_screen(int vx, int vy, int *sx, int *sy)
{
    *sx = (int)(vx * g_plat.scale_x) + g_plat.offset_x;
    *sy = (int)(vy * g_plat.scale_y) + g_plat.offset_y;
}

static void screen_to_virtual(int sx, int sy, int *vx, int *vy)
{
    *vx = (int)((sx - g_plat.offset_x) / g_plat.scale_x);
    *vy = (int)((sy - g_plat.offset_y) / g_plat.scale_y);
}

static void update_scale(void)
{
    float sx, sy;
    SDL_GetWindowSize(g_plat.window, &g_plat.window_w, &g_plat.window_h);

    sx = (float)g_plat.window_w / SCREEN_WIDTH;
    sy = (float)g_plat.window_h / SCREEN_HEIGHT;

    if (sx < sy) {
        g_plat.scale_x = sx;
        g_plat.scale_y = sx;
        g_plat.offset_x = 0;
        g_plat.offset_y = (g_plat.window_h - (int)(SCREEN_HEIGHT * sx)) / 2;
    } else {
        g_plat.scale_x = sy;
        g_plat.scale_y = sy;
        g_plat.offset_x = (g_plat.window_w - (int)(SCREEN_WIDTH * sy)) / 2;
        g_plat.offset_y = 0;
    }
}

/* ================================================================
 * PLATFORM LIFECYCLE
 * ================================================================ */

bool platform_init(const char *title, int width, int height)
{
    memset(&g_plat, 0, sizeof(PlatformSDL2));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio failed: %s\n", Mix_GetError());
        /* Audio failure is non-fatal */
    }
    Mix_AllocateChannels(MAX_CHANNELS);

    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        /* Font failure is non-fatal, will use fallback */
    }

    g_plat.window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!g_plat.window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    g_plat.renderer = SDL_CreateRenderer(
        g_plat.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!g_plat.renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawBlendMode(g_plat.renderer, SDL_BLENDMODE_BLEND);

    /* Load fonts (try common paths) */
    {
        const char *font_paths[] = {
            "assets/fonts/font.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "C:\\Windows\\Fonts\\arial.ttf",
            NULL
        };
        int fi;
        for (fi = 0; font_paths[fi] != NULL; fi++) {
            g_plat.font_small = TTF_OpenFont(font_paths[fi], 14);
            if (g_plat.font_small) {
                g_plat.font_medium = TTF_OpenFont(font_paths[fi], 24);
                g_plat.font_large = TTF_OpenFont(font_paths[fi], 48);
                break;
            }
        }
    }

    update_scale();
    return true;
}

void platform_shutdown(void)
{
    int i;

    for (i = 0; i < MAX_TEXTURES; i++) {
        if (g_plat.textures[i]) {
            SDL_DestroyTexture(g_plat.textures[i]);
        }
    }

    for (i = 0; i < MAX_SOUNDS; i++) {
        if (g_plat.sounds[i]) {
            Mix_FreeChunk(g_plat.sounds[i]);
        }
    }

    if (g_plat.font_small) TTF_CloseFont(g_plat.font_small);
    if (g_plat.font_medium) TTF_CloseFont(g_plat.font_medium);
    if (g_plat.font_large) TTF_CloseFont(g_plat.font_large);

    if (g_plat.renderer) SDL_DestroyRenderer(g_plat.renderer);
    if (g_plat.window) SDL_DestroyWindow(g_plat.window);

    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

bool platform_should_quit(void)
{
    return g_plat.quit_requested;
}

/* ================================================================
 * TIMING
 * ================================================================ */

uint32_t platform_get_ticks_ms(void)
{
    return SDL_GetTicks();
}

void platform_sleep_ms(uint32_t ms)
{
    SDL_Delay(ms);
}

/* ================================================================
 * INPUT
 * ================================================================ */

void platform_poll_input(void)
{
    SDL_Event event;
    int i;

    /* Clear per-frame flags */
    for (i = 0; i < BTN_COUNT; i++) {
        g_plat.keys_pressed[i] = false;
    }
    g_plat.mouse_clicked = false;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                g_plat.quit_requested = true;
                break;

            case SDL_KEYDOWN: {
                InputButton btn = sdl_key_to_button(event.key.keysym.sym);
                if (btn < BTN_COUNT) {
                    if (!event.key.repeat) {
                        g_plat.keys_pressed[btn] = true;
                    }
                    g_plat.keys_held[btn] = true;
                }
                break;
            }

            case SDL_KEYUP: {
                InputButton btn = sdl_key_to_button(event.key.keysym.sym);
                if (btn < BTN_COUNT) {
                    g_plat.keys_held[btn] = false;
                }
                break;
            }

            case SDL_MOUSEMOTION:
                screen_to_virtual(event.motion.x, event.motion.y,
                                  &g_plat.mouse_x, &g_plat.mouse_y);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    g_plat.mouse_down = true;
                    g_plat.mouse_clicked = true;
                    screen_to_virtual(event.button.x, event.button.y,
                                      &g_plat.mouse_x, &g_plat.mouse_y);
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    g_plat.mouse_down = false;
                }
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    update_scale();
                }
                break;
        }
    }
}

bool platform_button_pressed(InputButton btn)
{
    if (btn < 0 || btn >= BTN_COUNT) return false;
    return g_plat.keys_pressed[btn];
}

bool platform_button_held(InputButton btn)
{
    if (btn < 0 || btn >= BTN_COUNT) return false;
    return g_plat.keys_held[btn];
}

void platform_get_cursor(int *x, int *y)
{
    if (x) *x = g_plat.mouse_x;
    if (y) *y = g_plat.mouse_y;
}

bool platform_cursor_down(void)
{
    return g_plat.mouse_down;
}

bool platform_cursor_clicked(void)
{
    return g_plat.mouse_clicked;
}

/* ================================================================
 * RENDERING
 * ================================================================ */

void platform_render_begin(void)
{
    SDL_SetRenderDrawColor(g_plat.renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_plat.renderer);
}

void platform_render_end(void)
{
    SDL_RenderPresent(g_plat.renderer);
}

void platform_draw_sprite(int sprite_id, int x, int y)
{
    SDL_Rect dst;
    SDL_Texture *tex;

    if (sprite_id < 0 || sprite_id >= MAX_TEXTURES) return;
    tex = g_plat.textures[sprite_id];
    if (!tex) return;

    virtual_to_screen(x, y, &dst.x, &dst.y);
    dst.w = (int)(g_plat.tex_w[sprite_id] * g_plat.scale_x);
    dst.h = (int)(g_plat.tex_h[sprite_id] * g_plat.scale_y);

    SDL_RenderCopy(g_plat.renderer, tex, NULL, &dst);
}

void platform_draw_sprite_ex(int sprite_id, int x, int y,
                              int w, int h, uint8_t alpha,
                              bool flip_h, bool flip_v)
{
    SDL_Rect dst;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    SDL_Texture *tex;

    if (sprite_id < 0 || sprite_id >= MAX_TEXTURES) return;
    tex = g_plat.textures[sprite_id];
    if (!tex) return;

    virtual_to_screen(x, y, &dst.x, &dst.y);

    if (w > 0 && h > 0) {
        dst.w = (int)(w * g_plat.scale_x);
        dst.h = (int)(h * g_plat.scale_y);
    } else {
        dst.w = (int)(g_plat.tex_w[sprite_id] * g_plat.scale_x);
        dst.h = (int)(g_plat.tex_h[sprite_id] * g_plat.scale_y);
    }

    if (flip_h) flip |= SDL_FLIP_HORIZONTAL;
    if (flip_v) flip |= SDL_FLIP_VERTICAL;

    SDL_SetTextureAlphaMod(tex, alpha);
    SDL_RenderCopyEx(g_plat.renderer, tex, NULL, &dst, 0.0, NULL, flip);
    SDL_SetTextureAlphaMod(tex, 255);
}

void platform_draw_rect(int x, int y, int w, int h,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    SDL_Rect rect;
    virtual_to_screen(x, y, &rect.x, &rect.y);
    rect.w = (int)(w * g_plat.scale_x);
    rect.h = (int)(h * g_plat.scale_y);

    SDL_SetRenderDrawColor(g_plat.renderer, r, g, b, a);
    SDL_RenderFillRect(g_plat.renderer, &rect);
}

void platform_draw_text(const char *text, int x, int y,
                         uint8_t r, uint8_t g, uint8_t b, int size)
{
    TTF_Font *font;
    SDL_Color color;
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;

    switch (size) {
        case 2:  font = g_plat.font_large;  break;
        case 1:  font = g_plat.font_medium; break;
        default: font = g_plat.font_small;  break;
    }

    if (!font) {
        /* Fallback: draw a small rectangle for each character */
        int i, len;
        int char_w = (size == 2) ? 24 : (size == 1) ? 14 : 8;
        int char_h = (size == 2) ? 48 : (size == 1) ? 24 : 14;

        for (i = 0, len = 0; text[len]; len++) {}

        platform_draw_rect(x, y, len * char_w, char_h, r, g, b, 200);
        return;
    }

    color.r = r;
    color.g = g;
    color.b = b;
    color.a = 255;

    surf = TTF_RenderText_Blended(font, text, color);
    if (!surf) return;

    tex = SDL_CreateTextureFromSurface(g_plat.renderer, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        return;
    }

    virtual_to_screen(x, y, &dst.x, &dst.y);
    dst.w = (int)(surf->w * g_plat.scale_x);
    dst.h = (int)(surf->h * g_plat.scale_y);

    SDL_RenderCopy(g_plat.renderer, tex, NULL, &dst);

    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void platform_draw_text_centered(const char *text, int y,
                                  uint8_t r, uint8_t g, uint8_t b, int size)
{
    TTF_Font *font;
    int tw, th;

    switch (size) {
        case 2:  font = g_plat.font_large;  break;
        case 1:  font = g_plat.font_medium; break;
        default: font = g_plat.font_small;  break;
    }

    if (font && TTF_SizeText(font, text, &tw, &th) == 0) {
        platform_draw_text(text, (SCREEN_WIDTH - tw) / 2, y, r, g, b, size);
    } else {
        /* Fallback: estimate width */
        int char_w = (size == 2) ? 24 : (size == 1) ? 14 : 8;
        int len = 0;
        while (text[len]) len++;
        platform_draw_text(text, (SCREEN_WIDTH - len * char_w) / 2, y,
                          r, g, b, size);
    }
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
    Mix_Chunk *chunk;

    if (sound_id < 0 || sound_id >= MAX_SOUNDS) return -1;
    chunk = g_plat.sounds[sound_id];
    if (!chunk) return -1;

    return Mix_PlayChannel(-1, chunk, loop ? -1 : 0);
}

void platform_stop_sound(int handle)
{
    if (handle >= 0 && handle < MAX_CHANNELS) {
        Mix_HaltChannel(handle);
    }
}

void platform_stop_all_sounds(void)
{
    Mix_HaltChannel(-1);
    if (g_plat.music) {
        Mix_HaltMusic();
    }
}

void platform_set_volume(int handle, int volume)
{
    if (handle >= 0 && handle < MAX_CHANNELS) {
        Mix_Volume(handle, volume / 2); /* SDL mixer uses 0-128 */
    }
}

void platform_set_master_volume(int volume)
{
    Mix_MasterVolume(volume / 2);
}

bool platform_is_sound_playing(int handle)
{
    if (handle < 0 || handle >= MAX_CHANNELS) return false;
    return Mix_Playing(handle) != 0;
}

/* ================================================================
 * RESOURCES
 * ================================================================ */

bool platform_load_resource(int resource_id)
{
    const char *path;
    SDL_Surface *surf;

    if (resource_id < 0 || resource_id >= MAX_TEXTURES) return false;
    if (g_plat.textures[resource_id]) return true; /* Already loaded */

    path = resource_get_path(resource_id);
    if (!path) return false;

    surf = IMG_Load(path);
    if (!surf) {
        /* Non-fatal: game works without assets (shows placeholders) */
        return false;
    }

    g_plat.textures[resource_id] = SDL_CreateTextureFromSurface(
        g_plat.renderer, surf);
    g_plat.tex_w[resource_id] = surf->w;
    g_plat.tex_h[resource_id] = surf->h;

    SDL_SetTextureBlendMode(g_plat.textures[resource_id], SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surf);

    return g_plat.textures[resource_id] != NULL;
}

void platform_unload_resource(int resource_id)
{
    if (resource_id < 0 || resource_id >= MAX_TEXTURES) return;
    if (g_plat.textures[resource_id]) {
        SDL_DestroyTexture(g_plat.textures[resource_id]);
        g_plat.textures[resource_id] = NULL;
    }
}

void platform_load_resources_for_state(GameStateID state)
{
    (void)state;
    /* Handled by resource.c calling platform_load_resource() */
}

void platform_unload_resources_for_state(GameStateID state)
{
    (void)state;
}

bool platform_resource_loaded(int resource_id)
{
    if (resource_id < 0 || resource_id >= MAX_TEXTURES) return false;
    return g_plat.textures[resource_id] != NULL;
}

/* ================================================================
 * FILE I/O
 * ================================================================ */

bool platform_save_data(const void *data, size_t size)
{
    FILE *f = fopen(SAVE_FILENAME, "wb");
    if (!f) return false;

    if (fwrite(data, 1, size, f) != size) {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

bool platform_load_data(void *data, size_t size)
{
    FILE *f = fopen(SAVE_FILENAME, "rb");
    if (!f) return false;

    if (fread(data, 1, size, f) != size) {
        fclose(f);
        return false;
    }

    fclose(f);
    return true;
}

#endif /* FNAF2_PLATFORM_SDL2 */
