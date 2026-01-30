/*
 * music_box.c - Music box / Marionette defense system implementation
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Timer starts at 2000. Drains every 50ms at a rate depending on night:
 *   Night 1: 2, Night 2: 2, Night 3: 3, Night 4: 4, Night 5: 5, Night 6+: 6
 *
 * Winding the music box from Prize Corner camera replenishes the timer.
 * If the timer hits 0, the Marionette attacks.
 */

#include "music_box.h"
#include "platform.h"

#define WARNING_THRESHOLD   400     /* Start warning at 20% */

void music_box_init(MusicBox *mb)
{
    FNAF2_MEMSET(mb, 0, sizeof(MusicBox));
    mb->timer = MUSIC_BOX_TIMER_MAX;
    mb->timer_max = MUSIC_BOX_TIMER_MAX;
    mb->drain_rate = 2;
    mb->warning_handle = -1;
    mb->music_handle = -1;
}

void music_box_reset(MusicBox *mb, int drain_rate)
{
    mb->timer = MUSIC_BOX_TIMER_MAX;
    mb->timer_max = MUSIC_BOX_TIMER_MAX;
    mb->drain_rate = drain_rate;
    mb->drain_accumulator = 0;
    mb->is_winding = false;
    mb->has_run_out = false;
    mb->warning_playing = false;

    if (mb->warning_handle >= 0) {
        platform_stop_sound(mb->warning_handle);
        mb->warning_handle = -1;
    }
    if (mb->music_handle >= 0) {
        platform_stop_sound(mb->music_handle);
        mb->music_handle = -1;
    }

    /* Start music box tune */
    mb->music_handle = platform_play_sound(SND_MUSIC_BOX_TUNE, true);
}

void music_box_start_wind(MusicBox *mb)
{
    mb->is_winding = true;
}

void music_box_stop_wind(MusicBox *mb)
{
    mb->is_winding = false;
}

bool music_box_update(MusicBox *mb, uint32_t delta_ms)
{
    if (mb->has_run_out) return false;

    /* Winding replenishes timer */
    if (mb->is_winding) {
        mb->timer += MUSIC_BOX_WIND_RATE * (int)(delta_ms / TICK_RATE_MS);
        if (mb->timer > mb->timer_max) {
            mb->timer = mb->timer_max;
        }
        /* Reset warning if replenished enough */
        if (mb->timer > WARNING_THRESHOLD && mb->warning_playing) {
            mb->warning_playing = false;
            if (mb->warning_handle >= 0) {
                platform_stop_sound(mb->warning_handle);
                mb->warning_handle = -1;
            }
        }
        return false;
    }

    /* Drain timer every MUSIC_BOX_TICK_MS (50ms) */
    mb->drain_accumulator += delta_ms;
    while (mb->drain_accumulator >= MUSIC_BOX_TICK_MS) {
        mb->drain_accumulator -= MUSIC_BOX_TICK_MS;
        mb->timer -= mb->drain_rate;
    }

    /* Check for warning state */
    if (mb->timer <= WARNING_THRESHOLD && !mb->warning_playing) {
        mb->warning_playing = true;
        /* Play warning sound - the music slows/distorts */
    }

    /* Check for empty */
    if (mb->timer <= 0) {
        mb->timer = 0;
        mb->has_run_out = true;

        /* Stop music */
        if (mb->music_handle >= 0) {
            platform_stop_sound(mb->music_handle);
            mb->music_handle = -1;
        }
        platform_play_sound(SND_MUSIC_BOX_STOP, false);

        return true; /* Marionette attacks! */
    }

    return false;
}

fixed_t music_box_get_percent(const MusicBox *mb)
{
    if (mb->timer_max <= 0) return 0;
    return FIXED_DIV(INT_TO_FIXED(mb->timer), INT_TO_FIXED(mb->timer_max));
}

bool music_box_is_warning(const MusicBox *mb)
{
    return mb->warning_playing;
}

bool music_box_is_empty(const MusicBox *mb)
{
    return mb->has_run_out;
}
