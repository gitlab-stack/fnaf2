/*
 * game_defs.h - Game constants, enumerations, and common definitions
 *
 * Five Nights at Freddy's 2 - C Port
 */

#ifndef FNAF2_GAME_DEFS_H
#define FNAF2_GAME_DEFS_H

#include "types.h"

/* ================================================================
 * SCREEN / RESOLUTION
 * ================================================================ */
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT   768

/* ================================================================
 * TIMING CONSTANTS
 * ================================================================ */
#define TICK_RATE_MS            20      /* Fixed timestep: 50 updates/sec      */
#define TICKS_PER_SECOND        50
#define HOUR_DURATION_SEC       89      /* 1 game hour = 89 real seconds       */
#define HOUR_DURATION_MS        (HOUR_DURATION_SEC * 1000)
#define NIGHT_HOURS             6       /* 12 AM through 5 AM, then 6 AM       */
#define NIGHT_DURATION_MS       (HOUR_DURATION_MS * NIGHT_HOURS)

/* ================================================================
 * ANIMATRONIC IDENTIFIERS
 * ================================================================ */
typedef enum {
    ANIM_TOY_FREDDY = 0,
    ANIM_TOY_BONNIE,
    ANIM_TOY_CHICA,
    ANIM_MANGLE,
    ANIM_BALLOON_BOY,
    ANIM_WITHERED_FREDDY,
    ANIM_WITHERED_BONNIE,
    ANIM_WITHERED_CHICA,
    ANIM_WITHERED_FOXY,
    ANIM_MARIONETTE,
    ANIM_GOLDEN_FREDDY,
    ANIM_COUNT
} AnimatronicID;

/* ================================================================
 * LOCATIONS (cameras + special positions)
 * ================================================================ */
typedef enum {
    /* Cameras (viewable on the monitor) */
    CAM_PARTY_ROOM_1 = 0,  /* CAM 01 */
    CAM_PARTY_ROOM_2,       /* CAM 02 */
    CAM_PARTY_ROOM_3,       /* CAM 03 */
    CAM_PARTY_ROOM_4,       /* CAM 04 */
    CAM_LEFT_VENT,           /* CAM 05 */
    CAM_RIGHT_VENT,          /* CAM 06 */
    CAM_MAIN_HALL,           /* CAM 07 */
    CAM_PARTS_SERVICE,       /* CAM 08 */
    CAM_SHOW_STAGE,          /* CAM 09 */
    CAM_GAME_AREA,           /* CAM 10 */
    CAM_PRIZE_CORNER,        /* CAM 11 */
    CAM_KIDS_COVE,           /* CAM 12 */
    CAM_COUNT,

    /* Non-camera locations */
    LOC_OFFICE = CAM_COUNT,
    LOC_HALLWAY,             /* Visible from office with flashlight */
    LOC_LEFT_VENT_NEAR,      /* Left vent entrance (office side) */
    LOC_RIGHT_VENT_NEAR,     /* Right vent entrance (office side) */
    LOC_INACTIVE,            /* Animatronic not active yet */
    LOC_COUNT
} Location;

/* Check if a location is a viewable camera */
#define IS_CAMERA_LOC(loc) ((loc) >= 0 && (loc) < CAM_COUNT)

/* Camera display names */
static const char * const CAMERA_NAMES[CAM_COUNT] = {
    "CAM 01", "CAM 02", "CAM 03", "CAM 04",
    "CAM 05", "CAM 06", "CAM 07", "CAM 08",
    "CAM 09", "CAM 10", "CAM 11", "CAM 12"
};

static const char * const CAMERA_AREA_NAMES[CAM_COUNT] = {
    "Party Room 1",  "Party Room 2",  "Party Room 3",  "Party Room 4",
    "Left Air Vent", "Right Air Vent", "Main Hall",     "Parts/Service",
    "Show Stage",    "Game Area",      "Prize Corner",  "Kid's Cove"
};

/* ================================================================
 * GAME STATES
 * ================================================================ */
typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_LOADING,
    GAME_STATE_NIGHT_START,     /* "12 AM" / night intro screen */
    GAME_STATE_PHONE_CALL,      /* Phone guy talking */
    GAME_STATE_PLAYING,         /* Main gameplay */
    GAME_STATE_JUMPSCARE,       /* Animatronic attacking */
    GAME_STATE_STATIC,          /* Static after death */
    GAME_STATE_6AM,             /* Night complete! */
    GAME_STATE_PAYCHECK,        /* Night 5 paycheck */
    GAME_STATE_GAME_OVER,       /* Game over screen */
    GAME_STATE_CUSTOM_NIGHT,    /* Night 7 / custom night setup */
    GAME_STATE_QUIT
} GameStateID;

/* ================================================================
 * PLAYER VIEW
 * ================================================================ */
typedef enum {
    VIEW_OFFICE,    /* Normal office view */
    VIEW_CAMERA,    /* Camera monitor raised */
    VIEW_MASK       /* Wearing Freddy mask */
} PlayerView;

/* ================================================================
 * ANIMATRONIC AI CONSTANTS
 * ================================================================ */
#define AI_MAX_LEVEL            20
#define MOVEMENT_CHECK_MS       5000    /* AI movement opportunity every 5s */
#define MOVEMENT_RANDOM_MAX     20      /* Random range: 0 to 20 */

/* Music box */
#define MUSIC_BOX_TIMER_MAX     2000
#define MUSIC_BOX_TICK_MS       50      /* Check every 50ms */
#define MUSIC_BOX_WIND_RATE     30      /* Timer gained per wind tick */

/* Foxy specific */
#define FOXY_ATTACK_TIMER_BASE  500     /* Base attack timer (ms) */
#define FOXY_ATTACK_TIMER_RAND  500     /* Random additional (ms) */
#define FOXY_FLASH_THRESHOLD    50      /* Hallway attack timer after flash */
#define FOXY_D_INCREMENT_MS     1000    /* D value increases every 1 second */

/* Golden Freddy */
#define GOLDEN_FREDDY_HALLWAY_CHECK_MS  1000
#define GOLDEN_FREDDY_HALLWAY_CHANCE    10  /* 1 in 10 */
#define GOLDEN_FREDDY_STARE_LIMIT      100

/* Hallucinations */
#define SHADOW_BONNIE_CHANCE    1000000     /* 1 in 1,000,000 */
#define SHADOW_FREDDY_CHANCE    100         /* Random 0-1000, <= 100 */

/* Attack timing */
#define OFFICE_ATTACK_TIMER_MS  5000        /* Time before attack if no mask */
#define MASK_LEAVE_TIMER_MS     3000        /* Time for animatronic to leave w/ mask */
#define JUMPSCARE_DURATION_MS   2000        /* Duration of jumpscare animation */

/* Flashlight */
#define FLASHLIGHT_MAX_BATTERY  1000
#define FLASHLIGHT_DRAIN_RATE   2           /* Per tick when on */
#define FLASHLIGHT_RECHARGE_RATE 1          /* Per tick when off */

/* ================================================================
 * NIGHT DEFINITIONS
 * ================================================================ */
#define MAX_NIGHTS      7
#define CUSTOM_NIGHT    7   /* Night 7 = custom night */

/* ================================================================
 * RESOURCE IDENTIFIERS
 * ================================================================ */
typedef enum {
    /* Office views */
    RES_OFFICE_DARK = 0,
    RES_OFFICE_FLASHLIGHT,
    RES_OFFICE_LEFT_LIGHT,
    RES_OFFICE_RIGHT_LIGHT,
    RES_OFFICE_MASK,
    RES_OFFICE_FAN,

    /* Camera system */
    RES_CAMERA_BORDER,
    RES_CAMERA_MAP,
    RES_CAMERA_STATIC,
    RES_CAMERA_RECORDING,

    /* Camera feeds (base + camera index) */
    RES_CAM_FEED_BASE,
    RES_CAM_FEED_LAST = RES_CAM_FEED_BASE + CAM_COUNT - 1,

    /* Animatronic sprites on cameras (base + anim index * CAM_COUNT + cam) */
    RES_ANIM_CAM_BASE,

    /* Animatronic office sprites */
    RES_ANIM_OFFICE_BASE = RES_ANIM_CAM_BASE + (ANIM_COUNT * CAM_COUNT),

    /* Animatronic hallway/vent sprites */
    RES_ANIM_HALLWAY_BASE = RES_ANIM_OFFICE_BASE + ANIM_COUNT,
    RES_ANIM_LEFT_VENT_BASE = RES_ANIM_HALLWAY_BASE + ANIM_COUNT,
    RES_ANIM_RIGHT_VENT_BASE = RES_ANIM_LEFT_VENT_BASE + ANIM_COUNT,

    /* Jumpscares */
    RES_JUMPSCARE_BASE = RES_ANIM_RIGHT_VENT_BASE + ANIM_COUNT,
    RES_JUMPSCARE_LAST = RES_JUMPSCARE_BASE + ANIM_COUNT - 1,

    /* UI elements */
    RES_UI_CLOCK,
    RES_UI_NIGHT_TEXT,
    RES_UI_WIND_BUTTON,
    RES_UI_MASK_BUTTON,
    RES_UI_CAMERA_TOGGLE,
    RES_UI_VENT_LIGHT_BTN,
    RES_UI_FLASHLIGHT_BTN,

    /* Menu */
    RES_MENU_BACKGROUND,
    RES_MENU_TITLE,
    RES_MENU_NEWSPAPER,
    RES_MENU_STAR,

    /* Misc */
    RES_STATIC_OVERLAY,
    RES_PAYCHECK,
    RES_GAME_OVER,
    RES_6AM,

    RES_COUNT
} ResourceID;

/* ================================================================
 * SOUND IDENTIFIERS
 * ================================================================ */
typedef enum {
    SND_CAMERA_OPEN = 0,
    SND_CAMERA_CLOSE,
    SND_CAMERA_SWITCH,
    SND_MASK_ON,
    SND_MASK_OFF,
    SND_FLASHLIGHT_ON,
    SND_FLASHLIGHT_OFF,
    SND_VENT_LIGHT,
    SND_MUSIC_BOX_TUNE,
    SND_MUSIC_BOX_STOP,
    SND_JUMPSCARE,
    SND_STATIC,
    SND_AMBIENCE_1,
    SND_AMBIENCE_2,
    SND_BREATHING,
    SND_DEEP_LAUGH,
    SND_CLOCK_CHIME,
    SND_FAN_LOOP,
    SND_WIND,
    SND_CIRCUS,
    SND_PHONE_CALL_BASE,   /* + night index for each night's call */
    SND_COUNT = SND_PHONE_CALL_BASE + MAX_NIGHTS
} SoundID;

/* ================================================================
 * INPUT BUTTON MAPPING
 * ================================================================ */
typedef enum {
    BTN_CURSOR_CLICK = 0,   /* Mouse click / touch / A button */
    BTN_CAMERA_TOGGLE,       /* Toggle camera panel */
    BTN_MASK_TOGGLE,         /* Toggle Freddy mask */
    BTN_FLASHLIGHT,          /* Flashlight on/off */
    BTN_VENT_LIGHT_LEFT,     /* Left vent light */
    BTN_VENT_LIGHT_RIGHT,    /* Right vent light */
    BTN_CAM_LEFT,            /* Switch camera left */
    BTN_CAM_RIGHT,           /* Switch camera right */
    BTN_WIND_BOX,            /* Wind music box */
    BTN_PAUSE,
    BTN_ACCEPT,              /* Menu confirm */
    BTN_BACK,                /* Menu back */
    BTN_COUNT
} InputButton;

#endif /* FNAF2_GAME_DEFS_H */
