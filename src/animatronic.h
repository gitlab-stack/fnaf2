/*
 * animatronic.h - Animatronic AI system
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Handles all animatronic behavior: movement checks, pathfinding,
 * attack states, and special mechanics (Foxy, Marionette, Golden Freddy).
 */

#ifndef FNAF2_ANIMATRONIC_H
#define FNAF2_ANIMATRONIC_H

#include "types.h"
#include "game_defs.h"

/* ================================================================
 * ANIMATRONIC STATE
 * ================================================================ */

typedef enum {
    ANIM_STATE_INACTIVE,        /* Not yet active this night */
    ANIM_STATE_IDLE,            /* At a location, waiting for movement */
    ANIM_STATE_MOVING,          /* Transitioning between locations */
    ANIM_STATE_AT_DOOR,         /* At office entrance (hallway/vent) */
    ANIM_STATE_IN_OFFICE,       /* Inside the office */
    ANIM_STATE_ATTACKING,       /* Performing jumpscare */
    ANIM_STATE_STUNNED,         /* Stunned by flashlight/mask */
    ANIM_STATE_RETREATING       /* Leaving office after mask */
} AnimState;

/* Approach direction (how they enter the office) */
typedef enum {
    APPROACH_HALLWAY,       /* Through the main hallway */
    APPROACH_LEFT_VENT,     /* Through the left vent */
    APPROACH_RIGHT_VENT,    /* Through the right vent */
    APPROACH_SPECIAL        /* Special entry (Marionette, Golden Freddy) */
} ApproachType;

/* Individual animatronic data */
typedef struct {
    AnimatronicID   id;
    AnimState       state;
    Location        position;           /* Current location */
    int             ai_level;           /* Current AI level (0-20) */
    ApproachType    approach;           /* How this animatronic reaches the office */

    /* Timers (in milliseconds) */
    uint32_t        move_timer;         /* Time until next movement opportunity */
    uint32_t        attack_timer;       /* Time until attack from office */
    uint32_t        stun_timer;         /* Remaining stun duration */
    uint32_t        retreat_timer;      /* Time to leave after being mask-blocked */

    /* Foxy-specific */
    int             foxy_d_value;       /* Alterable Value D (increases over time) */
    uint32_t        foxy_d_timer;       /* Timer for D increment */
    int             foxy_attack_timer;  /* Foxy's hallway attack countdown (ms) */
    int             foxy_flash_count;   /* How many times flashed */

    /* Golden Freddy specific */
    uint32_t        gf_hallway_timer;   /* Timer for hallway appearance check */
    int             gf_stare_counter;   /* Stare counter (reaches 100 = death) */
    bool            gf_in_hallway;      /* Currently visible in hallway */
    bool            gf_in_office;       /* Currently visible in office */

    /* General flags */
    bool            is_active;          /* Whether this animatronic is active tonight */
    bool            can_be_masked;      /* Whether the mask works against this one */
    bool            visible_on_cam;     /* Currently visible on their camera */
    int             path_index;         /* Current index in movement path */
} Animatronic;

/* ================================================================
 * PATH DEFINITIONS
 * ================================================================ */

#define MAX_PATH_LENGTH 8

typedef struct {
    Location    steps[MAX_PATH_LENGTH];
    int         length;
} AnimPath;

/* ================================================================
 * FUNCTION DECLARATIONS
 * ================================================================ */

/* Initialize an animatronic to default state */
void animatronic_init(Animatronic *anim, AnimatronicID id);

/* Reset animatronic for a new night */
void animatronic_reset(Animatronic *anim);

/* Set AI level for the current hour */
void animatronic_set_ai(Animatronic *anim, int level);

/* Update animatronic AI (called every tick).
 * Returns the animatronic that should jumpscare, or ANIM_COUNT if none. */
AnimatronicID animatronic_update(Animatronic *anim, uint32_t delta_ms, RNG *rng,
                                  bool camera_up, bool mask_on,
                                  bool flashlight_on, bool vent_light_left,
                                  bool vent_light_right,
                                  int current_night,
                                  bool any_in_office);

/* Check if an animatronic is at a specific location */
bool animatronic_at_location(const Animatronic *anim, Location loc);

/* Get the animatronic's approach type */
ApproachType animatronic_get_approach(AnimatronicID id);

/* Get the path for an animatronic */
const AnimPath *animatronic_get_path(AnimatronicID id);

/* Get display name for an animatronic */
const char *animatronic_get_name(AnimatronicID id);

/* Stun an animatronic (flashlight/camera effects) */
void animatronic_stun(Animatronic *anim, uint32_t duration_ms);

/* Force an animatronic to retreat (mask success) */
void animatronic_retreat(Animatronic *anim);

/* Check if any animatronic from an array is in the office */
bool animatronics_any_in_office(const Animatronic *anims, int count);

/* Check if any animatronic is at a given location */
bool animatronics_any_at_location(const Animatronic *anims, int count, Location loc);

#endif /* FNAF2_ANIMATRONIC_H */
