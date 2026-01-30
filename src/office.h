/*
 * office.h - Office mechanics (flashlight, mask, vent lights)
 *
 * Five Nights at Freddy's 2 - C Port
 */

#ifndef FNAF2_OFFICE_H
#define FNAF2_OFFICE_H

#include "types.h"
#include "game_defs.h"

typedef struct {
    /* Flashlight */
    bool        flashlight_on;
    int         flashlight_battery;     /* 0 to FLASHLIGHT_MAX_BATTERY */
    bool        flashlight_disabled;    /* Disabled by Balloon Boy */

    /* Freddy Mask */
    bool        mask_on;
    uint32_t    mask_transition_timer;  /* Timer for putting on/taking off */
    bool        mask_transitioning;     /* In the process of equipping/removing */

    /* Vent lights */
    bool        vent_light_left;
    bool        vent_light_right;

    /* Player view */
    PlayerView  current_view;

    /* Office scroll position (looking left/right) */
    int         scroll_x;              /* Horizontal scroll offset */
    int         scroll_target;         /* Target scroll position */

    /* Foxy light counter for the current night */
    int         foxy_light_counter;
} Office;

/* Initialize office */
void office_init(Office *office);

/* Reset for new night */
void office_reset(Office *office);

/* Toggle flashlight */
void office_toggle_flashlight(Office *office);

/* Toggle Freddy mask */
void office_toggle_mask(Office *office);

/* Set vent lights */
void office_set_vent_light_left(Office *office, bool on);
void office_set_vent_light_right(Office *office, bool on);

/* Update office state (battery, transitions, scrolling) */
void office_update(Office *office, uint32_t delta_ms);

/* Check if player can use flashlight */
bool office_can_use_flashlight(const Office *office);

/* Get the current player view */
PlayerView office_get_view(const Office *office);

/* Disable flashlight (Balloon Boy effect) */
void office_disable_flashlight(Office *office);

#endif /* FNAF2_OFFICE_H */
