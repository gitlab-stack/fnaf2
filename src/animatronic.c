/*
 * animatronic.c - Animatronic AI system implementation
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Implements the movement check system:
 *   Every 5 seconds, generate random(0-20)+1.
 *   If AI level >= result, movement succeeds.
 *
 * Each animatronic has a defined path from start to office.
 * Special behaviors for Foxy, Marionette, and Golden Freddy.
 */

#include "animatronic.h"
#include "platform.h"

/* ================================================================
 * ANIMATRONIC PATHS
 * ================================================================
 * Each animatronic follows a specific route to the office.
 * The last entry is always LOC_OFFICE.
 */

static const AnimPath PATHS[ANIM_COUNT] = {
    /* Toy Freddy: Show Stage -> Game Area -> Main Hall -> Hallway -> Office */
    [ANIM_TOY_FREDDY] = {
        { CAM_SHOW_STAGE, CAM_GAME_AREA, CAM_MAIN_HALL, LOC_HALLWAY, LOC_OFFICE },
        5
    },
    /* Toy Bonnie: Show Stage -> Game Area -> Party Room 3 -> Party Room 4 -> Right Vent -> Office */
    [ANIM_TOY_BONNIE] = {
        { CAM_SHOW_STAGE, CAM_GAME_AREA, CAM_PARTY_ROOM_3, CAM_PARTY_ROOM_4,
          CAM_RIGHT_VENT, LOC_RIGHT_VENT_NEAR, LOC_OFFICE },
        7
    },
    /* Toy Chica: Show Stage -> Game Area -> Party Room 1 -> Party Room 2 -> Left Vent -> Office */
    [ANIM_TOY_CHICA] = {
        { CAM_SHOW_STAGE, CAM_GAME_AREA, CAM_PARTY_ROOM_1, CAM_PARTY_ROOM_2,
          CAM_LEFT_VENT, LOC_LEFT_VENT_NEAR, LOC_OFFICE },
        7
    },
    /* Mangle: Kid's Cove -> Game Area -> Party Room 1 -> Party Room 2 -> Right Vent -> Office */
    [ANIM_MANGLE] = {
        { CAM_KIDS_COVE, CAM_GAME_AREA, CAM_PARTY_ROOM_1, CAM_PARTY_ROOM_2,
          CAM_RIGHT_VENT, LOC_RIGHT_VENT_NEAR, LOC_OFFICE },
        7
    },
    /* Balloon Boy: Game Area -> Party Room 2 -> Left Vent -> Office */
    [ANIM_BALLOON_BOY] = {
        { CAM_GAME_AREA, CAM_PARTY_ROOM_2, CAM_LEFT_VENT, LOC_LEFT_VENT_NEAR, LOC_OFFICE },
        5
    },
    /* Withered Freddy: Parts/Service -> Main Hall -> Hallway -> Office */
    [ANIM_WITHERED_FREDDY] = {
        { CAM_PARTS_SERVICE, CAM_MAIN_HALL, LOC_HALLWAY, LOC_OFFICE },
        4
    },
    /* Withered Bonnie: Parts/Service -> Party Room 3 -> Left Vent -> Office */
    [ANIM_WITHERED_BONNIE] = {
        { CAM_PARTS_SERVICE, CAM_PARTY_ROOM_3, CAM_LEFT_VENT, LOC_LEFT_VENT_NEAR, LOC_OFFICE },
        5
    },
    /* Withered Chica: Parts/Service -> Party Room 4 -> Right Vent -> Office */
    [ANIM_WITHERED_CHICA] = {
        { CAM_PARTS_SERVICE, CAM_PARTY_ROOM_4, CAM_RIGHT_VENT, LOC_RIGHT_VENT_NEAR, LOC_OFFICE },
        5
    },
    /* Withered Foxy: Parts/Service -> Main Hall -> Hallway -> (attack) */
    [ANIM_WITHERED_FOXY] = {
        { CAM_PARTS_SERVICE, CAM_MAIN_HALL, LOC_HALLWAY, LOC_OFFICE },
        4
    },
    /* Marionette: Prize Corner -> Office (special: triggered by music box) */
    [ANIM_MARIONETTE] = {
        { CAM_PRIZE_CORNER, LOC_OFFICE },
        2
    },
    /* Golden Freddy: (special: random appearance) */
    [ANIM_GOLDEN_FREDDY] = {
        { LOC_INACTIVE, LOC_HALLWAY, LOC_OFFICE },
        3
    }
};

/* Display names */
static const char * const ANIM_NAMES[ANIM_COUNT] = {
    "Toy Freddy",
    "Toy Bonnie",
    "Toy Chica",
    "Mangle",
    "Balloon Boy",
    "W. Freddy",
    "W. Bonnie",
    "W. Chica",
    "W. Foxy",
    "Marionette",
    "Golden Freddy"
};

/* ================================================================
 * INITIALIZATION
 * ================================================================ */

void animatronic_init(Animatronic *anim, AnimatronicID id)
{
    FNAF2_MEMSET(anim, 0, sizeof(Animatronic));
    anim->id = id;
    anim->state = ANIM_STATE_INACTIVE;
    anim->position = LOC_INACTIVE;

    /* Set approach type */
    switch (id) {
        case ANIM_TOY_FREDDY:
        case ANIM_WITHERED_FREDDY:
        case ANIM_WITHERED_FOXY:
            anim->approach = APPROACH_HALLWAY;
            break;
        case ANIM_TOY_BONNIE:
        case ANIM_MANGLE:
        case ANIM_WITHERED_CHICA:
            anim->approach = APPROACH_RIGHT_VENT;
            break;
        case ANIM_TOY_CHICA:
        case ANIM_BALLOON_BOY:
        case ANIM_WITHERED_BONNIE:
            anim->approach = APPROACH_LEFT_VENT;
            break;
        case ANIM_MARIONETTE:
        case ANIM_GOLDEN_FREDDY:
            anim->approach = APPROACH_SPECIAL;
            break;
        default:
            anim->approach = APPROACH_HALLWAY;
            break;
    }

    /* Mask effectiveness */
    anim->can_be_masked = true;
    if (id == ANIM_WITHERED_FOXY || id == ANIM_MARIONETTE) {
        anim->can_be_masked = false;
    }
}

void animatronic_reset(Animatronic *anim)
{
    AnimatronicID id = anim->id;
    ApproachType approach = anim->approach;
    bool maskable = anim->can_be_masked;

    anim->state = ANIM_STATE_INACTIVE;
    anim->ai_level = 0;
    anim->path_index = 0;
    anim->move_timer = MOVEMENT_CHECK_MS;
    anim->attack_timer = 0;
    anim->stun_timer = 0;
    anim->retreat_timer = 0;
    anim->foxy_d_value = 0;
    anim->foxy_d_timer = 0;
    anim->foxy_attack_timer = 0;
    anim->foxy_flash_count = 0;
    anim->gf_hallway_timer = 0;
    anim->gf_stare_counter = 0;
    anim->gf_in_hallway = false;
    anim->gf_in_office = false;
    anim->is_active = false;
    anim->visible_on_cam = false;

    /* Restore non-resettable fields */
    anim->id = id;
    anim->approach = approach;
    anim->can_be_masked = maskable;

    /* Set starting position from path */
    if (PATHS[id].length > 0) {
        anim->position = PATHS[id].steps[0];
    }
}

void animatronic_set_ai(Animatronic *anim, int level)
{
    anim->ai_level = FNAF2_CLAMP(level, 0, AI_MAX_LEVEL);
    if (level > 0 && anim->state == ANIM_STATE_INACTIVE) {
        anim->state = ANIM_STATE_IDLE;
        anim->is_active = true;
    }
}

/* ================================================================
 * STANDARD MOVEMENT CHECK
 * ================================================================
 * Generate random(0-20) + 1. If AI >= result, movement succeeds.
 */
static bool movement_check_standard(int ai_level, RNG *rng)
{
    int roll;
    if (ai_level <= 0) return false;
    roll = (int)rng_range(rng, MOVEMENT_RANDOM_MAX + 1) + 1;  /* 1 to 21 */
    return ai_level >= roll;
}

/* ================================================================
 * FOXY MOVEMENT CHECK
 * ================================================================
 * (21 + random(5)) - D <= AI level
 */
static bool foxy_movement_check(int ai_level, int d_value, RNG *rng)
{
    int threshold;
    if (ai_level <= 0) return false;
    threshold = (21 + (int)rng_range(rng, 5)) - d_value;
    return ai_level >= threshold;
}

/* ================================================================
 * ADVANCE ALONG PATH
 * ================================================================ */
static void advance_on_path(Animatronic *anim)
{
    const AnimPath *path = &PATHS[anim->id];
    if (anim->path_index < path->length - 1) {
        anim->path_index++;
        anim->position = path->steps[anim->path_index];
    }
}

/* ================================================================
 * UPDATE: STANDARD ANIMATRONIC
 * ================================================================ */
static AnimatronicID update_standard(Animatronic *anim, uint32_t delta_ms, RNG *rng,
                                      bool camera_up, bool mask_on)
{
    /* Handle stun */
    if (anim->state == ANIM_STATE_STUNNED) {
        if (anim->stun_timer > delta_ms) {
            anim->stun_timer -= delta_ms;
        } else {
            anim->stun_timer = 0;
            anim->state = ANIM_STATE_IDLE;
        }
        return ANIM_COUNT;
    }

    /* Handle retreat */
    if (anim->state == ANIM_STATE_RETREATING) {
        if (anim->retreat_timer > delta_ms) {
            anim->retreat_timer -= delta_ms;
        } else {
            /* Return to starting position */
            anim->state = ANIM_STATE_IDLE;
            anim->path_index = 0;
            anim->position = PATHS[anim->id].steps[0];
            anim->retreat_timer = 0;
        }
        return ANIM_COUNT;
    }

    /* Handle in-office attack countdown */
    if (anim->state == ANIM_STATE_IN_OFFICE) {
        if (mask_on && anim->can_be_masked) {
            /* Mask is on - animatronic will leave */
            animatronic_retreat(anim);
            return ANIM_COUNT;
        }

        /* No mask - attack countdown */
        if (anim->attack_timer > delta_ms) {
            anim->attack_timer -= delta_ms;
        } else {
            /* Attack! */
            anim->state = ANIM_STATE_ATTACKING;
            return anim->id;
        }
        return ANIM_COUNT;
    }

    /* At vent entrance near office */
    if (anim->position == LOC_LEFT_VENT_NEAR || anim->position == LOC_RIGHT_VENT_NEAR) {
        anim->state = ANIM_STATE_AT_DOOR;

        /* Movement check to enter office */
        anim->move_timer += delta_ms;
        if (anim->move_timer >= MOVEMENT_CHECK_MS) {
            anim->move_timer = 0;
            if (movement_check_standard(anim->ai_level, rng)) {
                /* Check if camera is down (can only enter when camera is down) */
                if (!camera_up) {
                    advance_on_path(anim);
                    if (anim->position == LOC_OFFICE) {
                        anim->state = ANIM_STATE_IN_OFFICE;
                        anim->attack_timer = OFFICE_ATTACK_TIMER_MS;
                    }
                }
            }
        }
        return ANIM_COUNT;
    }

    /* At hallway */
    if (anim->position == LOC_HALLWAY) {
        anim->state = ANIM_STATE_AT_DOOR;
        anim->move_timer += delta_ms;
        if (anim->move_timer >= MOVEMENT_CHECK_MS) {
            anim->move_timer = 0;
            if (movement_check_standard(anim->ai_level, rng)) {
                if (!camera_up) {
                    advance_on_path(anim);
                    if (anim->position == LOC_OFFICE) {
                        anim->state = ANIM_STATE_IN_OFFICE;
                        anim->attack_timer = OFFICE_ATTACK_TIMER_MS;
                    }
                }
            }
        }
        return ANIM_COUNT;
    }

    /* Standard movement from camera locations */
    if (anim->state == ANIM_STATE_IDLE && IS_CAMERA_LOC(anim->position)) {
        anim->move_timer += delta_ms;
        if (anim->move_timer >= MOVEMENT_CHECK_MS) {
            anim->move_timer = 0;
            if (movement_check_standard(anim->ai_level, rng)) {
                advance_on_path(anim);
                anim->visible_on_cam = IS_CAMERA_LOC(anim->position);
            }
        }
    }

    return ANIM_COUNT;
}

/* ================================================================
 * UPDATE: WITHERED FOXY (Special behavior)
 * ================================================================
 * Movement: (21 + random(5)) - D <= AI level
 * D increases every second when no animatronics are in office.
 * After successful movement to hallway: attack timer starts.
 * Flashlight flashing resets counters.
 */
static AnimatronicID update_foxy(Animatronic *anim, uint32_t delta_ms, RNG *rng,
                                  bool flashlight_on, bool any_in_office,
                                  int current_night)
{
    if (anim->state == ANIM_STATE_STUNNED) {
        if (anim->stun_timer > delta_ms) {
            anim->stun_timer -= delta_ms;
        } else {
            anim->stun_timer = 0;
            anim->state = ANIM_STATE_IDLE;
        }
        return ANIM_COUNT;
    }

    /* Increment D value every second when no animatronics in office */
    if (!any_in_office) {
        anim->foxy_d_timer += delta_ms;
        if (anim->foxy_d_timer >= FOXY_D_INCREMENT_MS) {
            anim->foxy_d_timer -= FOXY_D_INCREMENT_MS;
            anim->foxy_d_value++;
        }
    }

    /* Foxy is in the hallway - handle attack */
    if (anim->position == LOC_HALLWAY) {
        if (flashlight_on) {
            /* Flashing the light at Foxy */
            anim->foxy_flash_count++;
            anim->foxy_attack_timer = FOXY_FLASH_THRESHOLD;

            /* Check if enough flashes to reset Foxy */
            if (anim->foxy_flash_count >= 100 * current_night) {
                /* Foxy retreats */
                anim->state = ANIM_STATE_IDLE;
                anim->path_index = 0;
                anim->position = PATHS[ANIM_WITHERED_FOXY].steps[0];
                anim->foxy_d_value = 0;
                anim->foxy_flash_count = 0;
                anim->foxy_attack_timer = 0;
                return ANIM_COUNT;
            }
        } else {
            /* Attack timer counting down */
            if (anim->foxy_attack_timer > 0) {
                if ((uint32_t)anim->foxy_attack_timer > delta_ms) {
                    anim->foxy_attack_timer -= (int)delta_ms;
                } else {
                    anim->foxy_attack_timer = 0;
                }
            }

            if (anim->foxy_attack_timer <= 0 && anim->state == ANIM_STATE_AT_DOOR) {
                /* Foxy attacks! */
                anim->state = ANIM_STATE_ATTACKING;
                return ANIM_WITHERED_FOXY;
            }
        }
        return ANIM_COUNT;
    }

    /* Standard movement check for Foxy (modified formula) */
    if (anim->state == ANIM_STATE_IDLE && IS_CAMERA_LOC(anim->position)) {
        anim->move_timer += delta_ms;
        if (anim->move_timer >= MOVEMENT_CHECK_MS) {
            anim->move_timer = 0;
            if (foxy_movement_check(anim->ai_level, anim->foxy_d_value, rng)) {
                advance_on_path(anim);
                if (anim->position == LOC_HALLWAY) {
                    anim->state = ANIM_STATE_AT_DOOR;
                    anim->foxy_attack_timer = FOXY_ATTACK_TIMER_BASE +
                                              (int)rng_range(rng, FOXY_ATTACK_TIMER_RAND);
                }
            }
        }
    }

    return ANIM_COUNT;
}

/* ================================================================
 * UPDATE: GOLDEN FREDDY (Special behavior)
 * ================================================================
 * Hallway: appears when AI > 0, flashlight off.
 *   Every second: if random(0-10)==1 and hallway empty -> appears.
 *   Flashlight exposure increases counter -> 100 = death.
 * Office: appears via movement check when camera is up.
 *   Pull up camera to dismiss.
 */
static AnimatronicID update_golden_freddy(Animatronic *anim, uint32_t delta_ms, RNG *rng,
                                           bool camera_up, bool mask_on,
                                           bool flashlight_on, bool hallway_occupied)
{
    FNAF2_UNUSED(mask_on);

    if (anim->ai_level <= 0) return ANIM_COUNT;

    /* Hallway appearance check */
    if (!anim->gf_in_hallway && !anim->gf_in_office) {
        anim->gf_hallway_timer += delta_ms;
        if (anim->gf_hallway_timer >= GOLDEN_FREDDY_HALLWAY_CHECK_MS) {
            anim->gf_hallway_timer = 0;
            if (!flashlight_on && !hallway_occupied) {
                if (rng_range(rng, GOLDEN_FREDDY_HALLWAY_CHANCE + 1) == 1) {
                    anim->gf_in_hallway = true;
                    anim->position = LOC_HALLWAY;
                }
            }
        }
    }

    /* Hallway stare - flashlight increases counter */
    if (anim->gf_in_hallway) {
        if (flashlight_on) {
            anim->gf_stare_counter += (int)(delta_ms / TICK_RATE_MS);
            if (anim->gf_stare_counter >= GOLDEN_FREDDY_STARE_LIMIT) {
                /* Golden Freddy death! */
                anim->state = ANIM_STATE_ATTACKING;
                return ANIM_GOLDEN_FREDDY;
            }
        } else {
            /* Dismiss if camera goes up */
            if (camera_up) {
                anim->gf_in_hallway = false;
                anim->gf_stare_counter = 0;
                anim->position = LOC_INACTIVE;
            }
        }
    }

    /* Office appearance - movement check when camera is up */
    if (camera_up && !anim->gf_in_hallway && !anim->gf_in_office) {
        anim->move_timer += delta_ms;
        if (anim->move_timer >= MOVEMENT_CHECK_MS) {
            anim->move_timer = 0;
            if (movement_check_standard(anim->ai_level, rng)) {
                anim->gf_in_office = true;
                anim->position = LOC_OFFICE;
                anim->state = ANIM_STATE_IN_OFFICE;
                anim->attack_timer = OFFICE_ATTACK_TIMER_MS;
            }
        }
    }

    /* Office - pull up camera to dismiss, otherwise attack */
    if (anim->gf_in_office) {
        if (camera_up) {
            /* Dismissed */
            anim->gf_in_office = false;
            anim->state = ANIM_STATE_IDLE;
            anim->position = LOC_INACTIVE;
            anim->attack_timer = 0;
        } else {
            if (anim->attack_timer > delta_ms) {
                anim->attack_timer -= delta_ms;
            } else {
                anim->state = ANIM_STATE_ATTACKING;
                return ANIM_GOLDEN_FREDDY;
            }
        }
    }

    return ANIM_COUNT;
}

/* ================================================================
 * MAIN UPDATE FUNCTION
 * ================================================================ */

AnimatronicID animatronic_update(Animatronic *anim, uint32_t delta_ms, RNG *rng,
                                  bool camera_up, bool mask_on,
                                  bool flashlight_on, bool vent_light_left,
                                  bool vent_light_right,
                                  int current_night,
                                  bool any_in_office)
{
    FNAF2_UNUSED(vent_light_left);
    FNAF2_UNUSED(vent_light_right);

    if (!anim->is_active || anim->state == ANIM_STATE_INACTIVE) {
        return ANIM_COUNT;
    }

    if (anim->state == ANIM_STATE_ATTACKING) {
        return anim->id;
    }

    switch (anim->id) {
        case ANIM_WITHERED_FOXY:
            return update_foxy(anim, delta_ms, rng, flashlight_on,
                              any_in_office, current_night);

        case ANIM_GOLDEN_FREDDY: {
            bool hallway_occupied = false;
            /* Golden Freddy checks if hallway is empty (other anims) -
             * this is handled by the caller via any_in_office for simplicity */
            return update_golden_freddy(anim, delta_ms, rng, camera_up, mask_on,
                                       flashlight_on, hallway_occupied);
        }

        case ANIM_MARIONETTE:
            /* Marionette is handled by the music box system.
             * When music box runs out, game.c sets marionette to attack. */
            return ANIM_COUNT;

        default:
            return update_standard(anim, delta_ms, rng, camera_up, mask_on);
    }
}

/* ================================================================
 * UTILITY FUNCTIONS
 * ================================================================ */

bool animatronic_at_location(const Animatronic *anim, Location loc)
{
    return anim->position == loc && anim->is_active;
}

ApproachType animatronic_get_approach(AnimatronicID id)
{
    switch (id) {
        case ANIM_TOY_FREDDY:
        case ANIM_WITHERED_FREDDY:
        case ANIM_WITHERED_FOXY:
            return APPROACH_HALLWAY;
        case ANIM_TOY_BONNIE:
        case ANIM_MANGLE:
        case ANIM_WITHERED_CHICA:
            return APPROACH_RIGHT_VENT;
        case ANIM_TOY_CHICA:
        case ANIM_BALLOON_BOY:
        case ANIM_WITHERED_BONNIE:
            return APPROACH_LEFT_VENT;
        default:
            return APPROACH_SPECIAL;
    }
}

const AnimPath *animatronic_get_path(AnimatronicID id)
{
    if (id >= 0 && id < ANIM_COUNT) {
        return &PATHS[id];
    }
    return NULL;
}

const char *animatronic_get_name(AnimatronicID id)
{
    if (id >= 0 && id < ANIM_COUNT) {
        return ANIM_NAMES[id];
    }
    return "Unknown";
}

void animatronic_stun(Animatronic *anim, uint32_t duration_ms)
{
    if (anim->state == ANIM_STATE_IDLE || anim->state == ANIM_STATE_AT_DOOR) {
        anim->state = ANIM_STATE_STUNNED;
        anim->stun_timer = duration_ms;
    }
}

void animatronic_retreat(Animatronic *anim)
{
    anim->state = ANIM_STATE_RETREATING;
    anim->retreat_timer = MASK_LEAVE_TIMER_MS;
    anim->attack_timer = 0;
}

bool animatronics_any_in_office(const Animatronic *anims, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        if (anims[i].state == ANIM_STATE_IN_OFFICE) {
            return true;
        }
    }
    return false;
}

bool animatronics_any_at_location(const Animatronic *anims, int count, Location loc)
{
    int i;
    for (i = 0; i < count; i++) {
        if (anims[i].is_active && anims[i].position == loc) {
            return true;
        }
    }
    return false;
}
