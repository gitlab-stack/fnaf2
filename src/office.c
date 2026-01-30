/*
 * office.c - Office mechanics implementation
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Handles the flashlight (with battery), Freddy mask,
 * vent lights, and office scrolling.
 */

#include "office.h"
#include "platform.h"

#define MASK_TRANSITION_MS      300     /* Time to put on/take off mask */
#define SCROLL_SPEED            8       /* Pixels per tick */
#define SCROLL_MAX              200     /* Maximum scroll offset */

void office_init(Office *office)
{
    FNAF2_MEMSET(office, 0, sizeof(Office));
    office->flashlight_battery = FLASHLIGHT_MAX_BATTERY;
    office->current_view = VIEW_OFFICE;
}

void office_reset(Office *office)
{
    office->flashlight_on = false;
    office->flashlight_battery = FLASHLIGHT_MAX_BATTERY;
    office->flashlight_disabled = false;
    office->mask_on = false;
    office->mask_transition_timer = 0;
    office->mask_transitioning = false;
    office->vent_light_left = false;
    office->vent_light_right = false;
    office->current_view = VIEW_OFFICE;
    office->scroll_x = 0;
    office->scroll_target = 0;
    office->foxy_light_counter = 0;
}

void office_toggle_flashlight(Office *office)
{
    if (office->flashlight_disabled) return;
    if (office->mask_on || office->mask_transitioning) return;
    if (office->current_view == VIEW_CAMERA) return;

    office->flashlight_on = !office->flashlight_on;

    if (office->flashlight_on) {
        platform_play_sound(SND_FLASHLIGHT_ON, false);
    } else {
        platform_play_sound(SND_FLASHLIGHT_OFF, false);
    }
}

void office_toggle_mask(Office *office)
{
    if (office->current_view == VIEW_CAMERA) return;
    if (office->mask_transitioning) return;

    office->mask_transitioning = true;
    office->mask_transition_timer = 0;

    if (office->mask_on) {
        platform_play_sound(SND_MASK_OFF, false);
    } else {
        /* Turn off flashlight when putting on mask */
        office->flashlight_on = false;
        platform_play_sound(SND_MASK_ON, false);
    }
}

void office_set_vent_light_left(Office *office, bool on)
{
    if (office->current_view == VIEW_CAMERA) return;
    office->vent_light_left = on;
    if (on) {
        platform_play_sound(SND_VENT_LIGHT, false);
    }
}

void office_set_vent_light_right(Office *office, bool on)
{
    if (office->current_view == VIEW_CAMERA) return;
    office->vent_light_right = on;
    if (on) {
        platform_play_sound(SND_VENT_LIGHT, false);
    }
}

void office_update(Office *office, uint32_t delta_ms)
{
    /* Handle mask transition */
    if (office->mask_transitioning) {
        office->mask_transition_timer += delta_ms;
        if (office->mask_transition_timer >= MASK_TRANSITION_MS) {
            office->mask_transitioning = false;
            office->mask_transition_timer = 0;
            office->mask_on = !office->mask_on;

            if (office->mask_on) {
                office->current_view = VIEW_MASK;
            } else {
                office->current_view = VIEW_OFFICE;
            }
        }
    }

    /* Flashlight battery */
    if (office->flashlight_on && !office->flashlight_disabled) {
        office->flashlight_battery -= FLASHLIGHT_DRAIN_RATE * (int)(delta_ms / TICK_RATE_MS);
        if (office->flashlight_battery <= 0) {
            office->flashlight_battery = 0;
            office->flashlight_on = false;
        }
    } else if (!office->flashlight_on) {
        /* Recharge when off */
        office->flashlight_battery += FLASHLIGHT_RECHARGE_RATE * (int)(delta_ms / TICK_RATE_MS);
        if (office->flashlight_battery > FLASHLIGHT_MAX_BATTERY) {
            office->flashlight_battery = FLASHLIGHT_MAX_BATTERY;
        }
    }

    /* Office scrolling (smooth interpolation) */
    if (office->scroll_x < office->scroll_target) {
        office->scroll_x += SCROLL_SPEED;
        if (office->scroll_x > office->scroll_target) {
            office->scroll_x = office->scroll_target;
        }
    } else if (office->scroll_x > office->scroll_target) {
        office->scroll_x -= SCROLL_SPEED;
        if (office->scroll_x < office->scroll_target) {
            office->scroll_x = office->scroll_target;
        }
    }
}

bool office_can_use_flashlight(const Office *office)
{
    return !office->flashlight_disabled &&
           !office->mask_on &&
           !office->mask_transitioning &&
           office->flashlight_battery > 0 &&
           office->current_view != VIEW_CAMERA;
}

PlayerView office_get_view(const Office *office)
{
    return office->current_view;
}

void office_disable_flashlight(Office *office)
{
    office->flashlight_disabled = true;
    office->flashlight_on = false;
}
