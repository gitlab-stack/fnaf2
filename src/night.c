/*
 * night.c - Night configuration and AI level tables
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * AI progression per night, per hour, per animatronic.
 * Based on the game mechanics document:
 *
 * Night 1: Toy animatronics start at 0, reach 2-3 by 2AM. Marionette=1.
 * Night 2: Toys reach 2-3 by 1AM. Golden Freddy gets random AI.
 *          Withered animatronics introduced.
 * Night 3: Toy Freddy absent. Marionette=8. Originals appear, Foxy=2.
 * Night 4: Golden Freddy odds improve. Toy Chica missing. Foxy=7.
 * Night 5: All return. Marionette=10. Foxy=5-7.
 * Night 6: Marionette=15. Foxy=10-15. Golden Freddy=3.
 * Night 7+: Same as Night 6 (custom night overrides).
 *
 * Hours: 0=12AM, 1=1AM, 2=2AM, 3=3AM, 4=4AM, 5=5AM
 */

#include "night.h"

/* ================================================================
 * AI LEVEL TABLES
 * ================================================================
 * Format: [animatronic][hour]
 * Indices: TOY_FREDDY, TOY_BONNIE, TOY_CHICA, MANGLE, BALLOON_BOY,
 *          W_FREDDY, W_BONNIE, W_CHICA, W_FOXY, MARIONETTE, GOLDEN_FREDDY
 */

/* Night 1: Easy introduction */
static const NightAIConfig NIGHT_1_AI = {{
    /* TOY_FREDDY */    { 0,  0,  2,  2,  3,  3 },
    /* TOY_BONNIE */    { 0,  0,  2,  3,  3,  3 },
    /* TOY_CHICA */     { 0,  0,  2,  2,  3,  3 },
    /* MANGLE */        { 0,  0,  0,  0,  0,  0 },
    /* BALLOON_BOY */   { 0,  0,  0,  0,  0,  0 },
    /* W_FREDDY */      { 0,  0,  0,  0,  0,  0 },
    /* W_BONNIE */      { 0,  0,  0,  0,  0,  0 },
    /* W_CHICA */       { 0,  0,  0,  0,  0,  0 },
    /* W_FOXY */        { 0,  0,  0,  0,  0,  0 },
    /* MARIONETTE */    { 1,  1,  1,  1,  1,  1 },
    /* GOLDEN_FREDDY */ { 0,  0,  0,  0,  0,  0 },
}};

/* Night 2: Toys more active, withered introduced */
static const NightAIConfig NIGHT_2_AI = {{
    /* TOY_FREDDY */    { 0,  2,  3,  3,  5,  5 },
    /* TOY_BONNIE */    { 0,  2,  3,  5,  5,  6 },
    /* TOY_CHICA */     { 0,  2,  3,  3,  5,  5 },
    /* MANGLE */        { 0,  0,  2,  3,  3,  4 },
    /* BALLOON_BOY */   { 0,  0,  1,  2,  2,  3 },
    /* W_FREDDY */      { 0,  0,  1,  2,  2,  3 },
    /* W_BONNIE */      { 0,  0,  1,  1,  2,  2 },
    /* W_CHICA */       { 0,  0,  1,  1,  2,  2 },
    /* W_FOXY */        { 0,  0,  0,  0,  0,  0 },
    /* MARIONETTE */    { 2,  2,  3,  3,  4,  4 },
    /* GOLDEN_FREDDY */ { 0,  0,  0,  1,  1,  1 },
}};

/* Night 3: No Toy Freddy. Originals appear. Foxy introduced. */
static const NightAIConfig NIGHT_3_AI = {{
    /* TOY_FREDDY */    { 0,  0,  0,  0,  0,  0 },
    /* TOY_BONNIE */    { 3,  4,  5,  6,  7,  7 },
    /* TOY_CHICA */     { 3,  3,  4,  5,  6,  7 },
    /* MANGLE */        { 2,  3,  4,  5,  6,  6 },
    /* BALLOON_BOY */   { 2,  3,  3,  4,  5,  5 },
    /* W_FREDDY */      { 1,  2,  3,  4,  5,  5 },
    /* W_BONNIE */      { 1,  2,  3,  3,  4,  5 },
    /* W_CHICA */       { 1,  2,  3,  3,  4,  5 },
    /* W_FOXY */        { 2,  2,  3,  3,  4,  5 },
    /* MARIONETTE */    { 8,  8,  8,  8,  8,  8 },
    /* GOLDEN_FREDDY */ { 0,  0,  1,  1,  1,  2 },
}};

/* Night 4: No Toy Chica. Foxy jumps to 7. */
static const NightAIConfig NIGHT_4_AI = {{
    /* TOY_FREDDY */    { 3,  4,  5,  6,  7,  8 },
    /* TOY_BONNIE */    { 3,  5,  6,  7,  8,  9 },
    /* TOY_CHICA */     { 0,  0,  0,  0,  0,  0 },
    /* MANGLE */        { 3,  5,  6,  7,  8,  9 },
    /* BALLOON_BOY */   { 3,  4,  5,  6,  7,  8 },
    /* W_FREDDY */      { 2,  3,  5,  6,  7,  8 },
    /* W_BONNIE */      { 2,  3,  4,  5,  6,  7 },
    /* W_CHICA */       { 2,  3,  4,  5,  6,  7 },
    /* W_FOXY */        { 7,  7,  8,  9, 10, 10 },
    /* MARIONETTE */    { 8,  8,  9, 10, 10, 10 },
    /* GOLDEN_FREDDY */ { 1,  1,  2,  2,  3,  3 },
}};

/* Night 5: All return. Marionette=10. Foxy=5-7. */
static const NightAIConfig NIGHT_5_AI = {{
    /* TOY_FREDDY */    { 5,  6,  7,  8,  9, 10 },
    /* TOY_BONNIE */    { 5,  7,  8,  9, 10, 11 },
    /* TOY_CHICA */     { 5,  6,  7,  8,  9, 10 },
    /* MANGLE */        { 5,  7,  8,  9, 10, 11 },
    /* BALLOON_BOY */   { 4,  5,  6,  7,  8,  9 },
    /* W_FREDDY */      { 4,  5,  6,  7,  8,  9 },
    /* W_BONNIE */      { 3,  4,  5,  6,  7,  8 },
    /* W_CHICA */       { 3,  4,  5,  6,  7,  8 },
    /* W_FOXY */        { 5,  5,  6,  6,  7,  7 },
    /* MARIONETTE */    {10, 10, 10, 10, 10, 10 },
    /* GOLDEN_FREDDY */ { 1,  2,  2,  3,  3,  4 },
}};

/* Night 6: Very aggressive. Marionette=15. Foxy=10-15. */
static const NightAIConfig NIGHT_6_AI = {{
    /* TOY_FREDDY */    {10, 11, 12, 13, 14, 15 },
    /* TOY_BONNIE */    {10, 12, 13, 14, 15, 16 },
    /* TOY_CHICA */     {10, 11, 12, 13, 14, 15 },
    /* MANGLE */        {10, 12, 13, 14, 15, 16 },
    /* BALLOON_BOY */   { 8, 10, 11, 12, 13, 14 },
    /* W_FREDDY */      { 8, 10, 11, 12, 13, 14 },
    /* W_BONNIE */      { 7,  9, 10, 11, 12, 13 },
    /* W_CHICA */       { 7,  9, 10, 11, 12, 13 },
    /* W_FOXY */        {10, 11, 12, 13, 14, 15 },
    /* MARIONETTE */    {15, 15, 15, 15, 15, 15 },
    /* GOLDEN_FREDDY */ { 3,  3,  4,  4,  5,  5 },
}};

/* Night configs array */
static const NightConfig NIGHT_CONFIGS[MAX_NIGHTS] = {
    { 1, 2, {{{0}}} },   /* Placeholder - filled at access time */
    { 2, 2, {{{0}}} },
    { 3, 3, {{{0}}} },
    { 4, 4, {{{0}}} },
    { 5, 5, {{{0}}} },
    { 6, 6, {{{0}}} },
    { 7, 6, {{{0}}} },   /* Night 7 uses night 6 drain rate */
};

/* Pointers to actual AI tables */
static const NightAIConfig * const AI_TABLES[MAX_NIGHTS] = {
    &NIGHT_1_AI,
    &NIGHT_2_AI,
    &NIGHT_3_AI,
    &NIGHT_4_AI,
    &NIGHT_5_AI,
    &NIGHT_6_AI,
    &NIGHT_6_AI     /* Night 7 defaults to night 6 */
};

/* Music box drain rates per night */
static const int MUSIC_BOX_DRAIN[MAX_NIGHTS] = {
    2, 2, 3, 4, 5, 6, 6
};

/* Custom night AI levels */
static int custom_ai_levels[ANIM_COUNT] = {0};

/* ================================================================
 * FUNCTIONS
 * ================================================================ */

const NightConfig *night_get_config(int night_number)
{
    if (night_number < 1) night_number = 1;
    if (night_number > MAX_NIGHTS) night_number = MAX_NIGHTS;
    return &NIGHT_CONFIGS[night_number - 1];
}

int night_get_ai_level(int night_number, AnimatronicID anim, int hour)
{
    const NightAIConfig *ai;

    if (night_number < 1) night_number = 1;
    if (night_number > MAX_NIGHTS) night_number = MAX_NIGHTS;
    if (anim < 0 || anim >= ANIM_COUNT) return 0;
    if (hour < 0) hour = 0;
    if (hour >= NIGHT_HOURS) hour = NIGHT_HOURS - 1;

    /* Custom night uses custom AI */
    if (night_number == CUSTOM_NIGHT) {
        return custom_ai_levels[anim];
    }

    ai = AI_TABLES[night_number - 1];
    return ai->ai_levels[anim][hour];
}

int night_get_music_box_drain(int night_number)
{
    if (night_number < 1) night_number = 1;
    if (night_number > MAX_NIGHTS) night_number = MAX_NIGHTS;
    return MUSIC_BOX_DRAIN[night_number - 1];
}

void night_set_custom_ai(int anim_id, int level)
{
    if (anim_id < 0 || anim_id >= ANIM_COUNT) return;
    custom_ai_levels[anim_id] = FNAF2_CLAMP(level, 0, AI_MAX_LEVEL);
}

int night_get_custom_ai(int anim_id)
{
    if (anim_id < 0 || anim_id >= ANIM_COUNT) return 0;
    return custom_ai_levels[anim_id];
}

/* ================================================================
 * SAVE DATA
 * ================================================================ */

void save_data_init(SaveData *save)
{
    FNAF2_MEMSET(save, 0, sizeof(SaveData));
    save->nights_completed = 0;
    save->stars = 0;
    save->night6_unlocked = false;
    save->night7_unlocked = false;
    save->checksum = save_data_checksum(save);
}

uint32_t save_data_checksum(const SaveData *save)
{
    /* Simple checksum: XOR and rotate fields */
    uint32_t cs = 0xDEAD2FAF;
    cs ^= (uint32_t)save->nights_completed * 31;
    cs ^= (uint32_t)save->stars * 127;
    cs ^= save->night6_unlocked ? 0x5A5A5A5A : 0;
    cs ^= save->night7_unlocked ? 0xA5A5A5A5 : 0;
    return cs;
}

bool save_data_valid(const SaveData *save)
{
    SaveData temp = *save;
    temp.checksum = 0;
    return save->checksum == save_data_checksum(&temp);
}
