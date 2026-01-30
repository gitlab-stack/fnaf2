/*
 * music_box.h - Music box / Marionette defense system
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * The music box must be wound from the Prize Corner camera.
 * If it runs out, the Marionette attacks and cannot be stopped.
 */

#ifndef FNAF2_MUSIC_BOX_H
#define FNAF2_MUSIC_BOX_H

#include "types.h"
#include "game_defs.h"

typedef struct {
    int         timer;              /* Current timer value (0 = out!) */
    int         timer_max;          /* Maximum timer value */
    int         drain_rate;         /* How fast it drains (per tick interval) */
    uint32_t    drain_accumulator;  /* Accumulator for drain timing */
    bool        is_winding;         /* Player is currently winding */
    bool        has_run_out;        /* Timer has hit zero */
    bool        warning_playing;    /* Low timer warning sound */
    int         warning_handle;     /* Audio handle for warning */
    int         music_handle;       /* Audio handle for music box tune */
} MusicBox;

/* Initialize music box */
void music_box_init(MusicBox *mb);

/* Reset for a new night with the given drain rate */
void music_box_reset(MusicBox *mb, int drain_rate);

/* Start winding the music box (player holding wind button on prize corner) */
void music_box_start_wind(MusicBox *mb);

/* Stop winding */
void music_box_stop_wind(MusicBox *mb);

/* Update music box timer.
 * Returns true if the timer just ran out (Marionette attacks). */
bool music_box_update(MusicBox *mb, uint32_t delta_ms);

/* Get timer as a percentage (0.0 to 1.0) using fixed-point */
fixed_t music_box_get_percent(const MusicBox *mb);

/* Check if music box is in danger (low timer) */
bool music_box_is_warning(const MusicBox *mb);

/* Check if music box has completely run out */
bool music_box_is_empty(const MusicBox *mb);

#endif /* FNAF2_MUSIC_BOX_H */
