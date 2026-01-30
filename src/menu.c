/*
 * menu.c - Menu system implementation
 *
 * Five Nights at Freddy's 2 - C Port
 */

#include "menu.h"
#include "animatronic.h"
#include "platform.h"

#define MENU_ANIM_SPEED     500     /* ms per star animation frame */
#define NEWSPAPER_DURATION  5000    /* ms to show newspaper */

void menu_init(Menu *menu)
{
    FNAF2_MEMSET(menu, 0, sizeof(Menu));
    menu->current_screen = MENU_MAIN;
    menu->selected_item = 0;
    menu->max_items = 2; /* New Game, Continue */
    save_data_init(&menu->save);
}

void menu_load_save(Menu *menu)
{
    SaveData loaded;
    if (platform_load_data(&loaded, sizeof(SaveData))) {
        if (save_data_valid(&loaded)) {
            menu->save = loaded;
        }
    }

    /* Update available menu items based on save */
    menu->max_items = 2; /* New Game, Continue */
    if (menu->save.night6_unlocked) {
        menu->max_items = 3;
    }
    if (menu->save.night7_unlocked) {
        menu->max_items = 4;
    }
}

int menu_update(Menu *menu, uint32_t delta_ms)
{
    menu->star_anim_timer += delta_ms;
    if (menu->star_anim_timer >= MENU_ANIM_SPEED) {
        menu->star_anim_timer -= MENU_ANIM_SPEED;
    }

    /* Handle newspaper display */
    if (menu->show_newspaper) {
        menu->newspaper_timer += delta_ms;
        if (menu->newspaper_timer >= NEWSPAPER_DURATION ||
            platform_button_pressed(BTN_ACCEPT)) {
            menu->show_newspaper = false;
            menu->newspaper_timer = 0;
        }
        return 0;
    }

    switch (menu->current_screen) {
        case MENU_MAIN: {
            /* Navigation */
            if (platform_button_pressed(BTN_CAM_LEFT) ||
                platform_button_pressed(BTN_CAMERA_TOGGLE)) {
                menu->selected_item--;
                if (menu->selected_item < 0) {
                    menu->selected_item = menu->max_items - 1;
                }
            }
            if (platform_button_pressed(BTN_CAM_RIGHT)) {
                menu->selected_item++;
                if (menu->selected_item >= menu->max_items) {
                    menu->selected_item = 0;
                }
            }

            /* Also handle cursor click on menu items */
            if (platform_cursor_clicked()) {
                int mx, my;
                int item;
                platform_get_cursor(&mx, &my);
                /* Check if cursor is over a menu item */
                /* Menu items are centered vertically, spaced 60px apart */
                item = (my - 400) / 60;
                if (item >= 0 && item < menu->max_items) {
                    menu->selected_item = item;
                    /* Fall through to accept */
                } else {
                    break;
                }
            } else if (!platform_button_pressed(BTN_ACCEPT)) {
                break;
            }

            /* Accept selection */
            switch (menu->selected_item) {
                case MENU_ITEM_NEW_GAME:
                    return 1; /* Start night 1 */
                case MENU_ITEM_CONTINUE:
                    return FNAF2_MAX(1, menu->save.nights_completed + 1);
                case MENU_ITEM_NIGHT_6:
                    return 6;
                case MENU_ITEM_CUSTOM_NIGHT:
                    menu->current_screen = MENU_CUSTOM_NIGHT;
                    menu->custom_selected = 0;
                    break;
            }
            break;
        }

        case MENU_CUSTOM_NIGHT: {
            /* Navigate animatronics */
            if (platform_button_pressed(BTN_CAM_LEFT)) {
                menu->custom_selected--;
                if (menu->custom_selected < 0) {
                    menu->custom_selected = ANIM_COUNT - 1;
                }
            }
            if (platform_button_pressed(BTN_CAM_RIGHT)) {
                menu->custom_selected++;
                if (menu->custom_selected >= ANIM_COUNT) {
                    menu->custom_selected = 0;
                }
            }

            /* Adjust AI level */
            if (platform_button_pressed(BTN_CAMERA_TOGGLE)) {
                int lvl = night_get_custom_ai(menu->custom_selected);
                lvl++;
                if (lvl > AI_MAX_LEVEL) lvl = 0;
                night_set_custom_ai(menu->custom_selected, lvl);
                menu->custom_ai[menu->custom_selected] = lvl;
            }
            if (platform_button_pressed(BTN_MASK_TOGGLE)) {
                int lvl = night_get_custom_ai(menu->custom_selected);
                lvl--;
                if (lvl < 0) lvl = AI_MAX_LEVEL;
                night_set_custom_ai(menu->custom_selected, lvl);
                menu->custom_ai[menu->custom_selected] = lvl;
            }

            /* Start custom night */
            if (platform_button_pressed(BTN_ACCEPT)) {
                return CUSTOM_NIGHT;
            }

            /* Back */
            if (platform_button_pressed(BTN_BACK)) {
                menu->current_screen = MENU_MAIN;
            }
            break;
        }

        default:
            break;
    }

    return 0;
}

void menu_render(const Menu *menu)
{
    int i;

    /* Background */
    platform_draw_sprite(RES_MENU_BACKGROUND, 0, 0);

    if (menu->show_newspaper) {
        platform_draw_sprite(RES_MENU_NEWSPAPER, 0, 0);
        return;
    }

    switch (menu->current_screen) {
        case MENU_MAIN:
            /* Title */
            platform_draw_text_centered("Five Nights at Freddy's 2", 150,
                                        255, 255, 255, 2);

            /* Menu items */
            {
                const char *items[] = {
                    "New Game", "Continue", "Night 6", "Custom Night"
                };
                for (i = 0; i < menu->max_items; i++) {
                    uint8_t r = 200, g = 200, b = 200;
                    if (i == menu->selected_item) {
                        r = 255; g = 255; b = 0;
                    }
                    platform_draw_text_centered(items[i], 400 + i * 60,
                                                r, g, b, 1);
                }
            }

            /* Stars */
            for (i = 0; i < menu->save.stars; i++) {
                platform_draw_sprite(RES_MENU_STAR, 400 + i * 50, 700);
            }
            break;

        case MENU_CUSTOM_NIGHT:
            platform_draw_text_centered("Custom Night", 50, 255, 255, 255, 2);

            /* Animatronic AI sliders */
            for (i = 0; i < ANIM_COUNT; i++) {
                int x = 100 + (i % 4) * 220;
                int y = 150 + (i / 4) * 180;
                uint8_t r = 200, g = 200, b = 200;
                char buf[32];
                int len;

                if (i == menu->custom_selected) {
                    r = 255; g = 255; b = 0;
                }

                platform_draw_text(animatronic_get_name(i), x, y, r, g, b, 0);

                /* AI level display */
                len = 0;
                buf[len++] = '0' + (menu->custom_ai[i] / 10);
                buf[len++] = '0' + (menu->custom_ai[i] % 10);
                buf[len] = '\0';
                platform_draw_text(buf, x + 50, y + 30, 255, 255, 255, 1);
            }

            platform_draw_text_centered("Press ACCEPT to start", 700,
                                        180, 180, 180, 0);
            break;

        default:
            break;
    }
}

void menu_complete_night(Menu *menu, int night)
{
    if (night > menu->save.nights_completed) {
        menu->save.nights_completed = night;
    }

    switch (night) {
        case 5:
            menu->save.night6_unlocked = true;
            menu->show_newspaper = true;
            menu->newspaper_timer = 0;
            menu->save.stars++;
            break;
        case 6:
            menu->save.night7_unlocked = true;
            menu->show_newspaper = true;
            menu->newspaper_timer = 0;
            menu->save.stars++;
            break;
        case 7:
            menu->save.stars++;
            break;
        default:
            break;
    }

    /* Clamp stars */
    if (menu->save.stars > 3) menu->save.stars = 3;

    /* Save */
    menu->save.checksum = save_data_checksum(&menu->save);
    platform_save_data(&menu->save, sizeof(SaveData));

    /* Update menu items */
    menu->max_items = 2;
    if (menu->save.night6_unlocked) menu->max_items = 3;
    if (menu->save.night7_unlocked) menu->max_items = 4;
}
