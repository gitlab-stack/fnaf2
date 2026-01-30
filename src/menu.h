/*
 * menu.h - Menu system
 *
 * Five Nights at Freddy's 2 - C Port
 */

#ifndef FNAF2_MENU_H
#define FNAF2_MENU_H

#include "types.h"
#include "game_defs.h"
#include "night.h"

typedef enum {
    MENU_MAIN,
    MENU_NIGHT_SELECT,
    MENU_CUSTOM_NIGHT,
    MENU_OPTIONS,
    MENU_COUNT
} MenuScreen;

typedef enum {
    MENU_ITEM_NEW_GAME = 0,
    MENU_ITEM_CONTINUE,
    MENU_ITEM_NIGHT_6,
    MENU_ITEM_CUSTOM_NIGHT,
    MENU_ITEM_COUNT
} MainMenuItem;

typedef struct {
    MenuScreen      current_screen;
    int             selected_item;
    int             max_items;
    SaveData        save;
    int             custom_ai[ANIM_COUNT];  /* Custom night AI levels */
    int             custom_selected;        /* Which animatronic is selected */
    uint32_t        star_anim_timer;        /* Timer for star animation */
    bool            show_newspaper;         /* Show night 5/6 newspaper */
    uint32_t        newspaper_timer;
} Menu;

/* Initialize menu */
void menu_init(Menu *menu);

/* Load save data into menu */
void menu_load_save(Menu *menu);

/* Update menu logic (handles input) */
/* Returns: night number to start (1-7), or 0 if still in menu, or -1 for quit */
int menu_update(Menu *menu, uint32_t delta_ms);

/* Render menu */
void menu_render(const Menu *menu);

/* Mark a night as completed */
void menu_complete_night(Menu *menu, int night);

#endif /* FNAF2_MENU_H */
