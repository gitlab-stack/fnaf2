/*
 * night.h - Night configuration and AI level tables
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Defines AI levels for each animatronic at each hour of each night,
 * as well as music box drain rates and other per-night settings.
 */

#ifndef FNAF2_NIGHT_H
#define FNAF2_NIGHT_H

#include "types.h"
#include "game_defs.h"

/* AI levels for each hour of a night (12AM through 5AM = 6 entries) */
typedef struct {
    int ai_levels[ANIM_COUNT][NIGHT_HOURS];
} NightAIConfig;

/* Per-night configuration */
typedef struct {
    int             night_number;       /* 1-7 */
    int             music_box_drain;    /* Music box drain rate */
    NightAIConfig   ai_config;          /* AI levels per hour */
} NightConfig;

/* Get the configuration for a specific night (1-7).
 * Night 7 (custom) uses provided AI levels. */
const NightConfig *night_get_config(int night_number);

/* Get AI level for a specific animatronic at a specific hour of a night */
int night_get_ai_level(int night_number, AnimatronicID anim, int hour);

/* Get music box drain rate for a night */
int night_get_music_box_drain(int night_number);

/* Custom night: set AI levels manually */
void night_set_custom_ai(int anim_id, int level);

/* Get custom night AI level */
int night_get_custom_ai(int anim_id);

/* ================================================================
 * SAVE DATA
 * ================================================================ */
typedef struct {
    int     nights_completed;       /* Highest night completed (0-7) */
    int     stars;                  /* Stars earned (0-3) */
    bool    night6_unlocked;        /* Beat night 5 */
    bool    night7_unlocked;        /* Beat night 6 */
    uint32_t checksum;              /* Simple data integrity check */
} SaveData;

/* Initialize save data to defaults */
void save_data_init(SaveData *save);

/* Calculate checksum for save data */
uint32_t save_data_checksum(const SaveData *save);

/* Validate save data integrity */
bool save_data_valid(const SaveData *save);

#endif /* FNAF2_NIGHT_H */
