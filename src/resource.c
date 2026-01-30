/*
 * resource.c - Resource/asset management implementation
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Maps resource IDs to file paths. The platform layer handles
 * actual file loading based on these paths.
 *
 * Asset directory structure expected:
 *   assets/
 *     office/         - Office view images
 *     cameras/        - Camera feed images
 *     animatronics/   - Animatronic sprites
 *     jumpscares/     - Jumpscare animations
 *     ui/             - UI elements
 *     menu/           - Menu assets
 *     audio/          - Sound effects and music
 */

#include "resource.h"
#include "platform.h"

/*
 * Resource path table.
 * On platforms with different path conventions, the platform layer
 * can prepend/translate these paths as needed.
 */
static const ResourceMapping RESOURCE_PATHS[] = {
    /* Office views */
    { RES_OFFICE_DARK,         "assets/office/office_dark.png" },
    { RES_OFFICE_FLASHLIGHT,   "assets/office/office_flashlight.png" },
    { RES_OFFICE_LEFT_LIGHT,   "assets/office/office_left_light.png" },
    { RES_OFFICE_RIGHT_LIGHT,  "assets/office/office_right_light.png" },
    { RES_OFFICE_MASK,         "assets/office/office_mask.png" },
    { RES_OFFICE_FAN,          "assets/office/office_fan.png" },

    /* Camera system */
    { RES_CAMERA_BORDER,       "assets/ui/camera_border.png" },
    { RES_CAMERA_MAP,          "assets/ui/camera_map.png" },
    { RES_CAMERA_STATIC,       "assets/ui/camera_static.png" },
    { RES_CAMERA_RECORDING,    "assets/ui/camera_rec.png" },

    /* UI */
    { RES_UI_CLOCK,            "assets/ui/clock.png" },
    { RES_UI_NIGHT_TEXT,       "assets/ui/night_text.png" },
    { RES_UI_WIND_BUTTON,     "assets/ui/wind_button.png" },
    { RES_UI_MASK_BUTTON,     "assets/ui/mask_button.png" },
    { RES_UI_CAMERA_TOGGLE,   "assets/ui/camera_toggle.png" },
    { RES_UI_VENT_LIGHT_BTN,  "assets/ui/vent_light.png" },
    { RES_UI_FLASHLIGHT_BTN,  "assets/ui/flashlight_btn.png" },

    /* Menu */
    { RES_MENU_BACKGROUND,     "assets/menu/background.png" },
    { RES_MENU_TITLE,          "assets/menu/title.png" },
    { RES_MENU_NEWSPAPER,      "assets/menu/newspaper.png" },
    { RES_MENU_STAR,           "assets/menu/star.png" },

    /* Misc */
    { RES_STATIC_OVERLAY,      "assets/ui/static_overlay.png" },
    { RES_PAYCHECK,            "assets/ui/paycheck.png" },
    { RES_GAME_OVER,           "assets/ui/game_over.png" },
    { RES_6AM,                 "assets/ui/6am.png" },

    /* Sentinel */
    { -1, NULL }
};

const char *resource_get_path(int resource_id)
{
    const ResourceMapping *mapping = RESOURCE_PATHS;
    while (mapping->path != NULL) {
        if (mapping->id == resource_id) {
            return mapping->path;
        }
        mapping++;
    }

    /*
     * Dynamic resource paths (cameras, animatronics, jumpscares)
     * are generated procedurally. The platform layer should handle
     * these based on the resource ID ranges defined in game_defs.h.
     */
    return NULL;
}

void resource_load_gameplay(void)
{
    int i;

    /* Load office resources */
    platform_load_resource(RES_OFFICE_DARK);
    platform_load_resource(RES_OFFICE_FLASHLIGHT);
    platform_load_resource(RES_OFFICE_LEFT_LIGHT);
    platform_load_resource(RES_OFFICE_RIGHT_LIGHT);
    platform_load_resource(RES_OFFICE_MASK);

    /* Load camera system resources */
    platform_load_resource(RES_CAMERA_BORDER);
    platform_load_resource(RES_CAMERA_MAP);
    platform_load_resource(RES_CAMERA_STATIC);
    platform_load_resource(RES_CAMERA_RECORDING);

    /* Load camera feeds */
    for (i = 0; i < CAM_COUNT; i++) {
        platform_load_resource(RES_CAM_FEED_BASE + i);
    }

    /* Load UI */
    platform_load_resource(RES_UI_CLOCK);
    platform_load_resource(RES_UI_NIGHT_TEXT);
    platform_load_resource(RES_UI_WIND_BUTTON);
    platform_load_resource(RES_UI_MASK_BUTTON);
    platform_load_resource(RES_UI_CAMERA_TOGGLE);
    platform_load_resource(RES_UI_VENT_LIGHT_BTN);
    platform_load_resource(RES_UI_FLASHLIGHT_BTN);

    /* Load misc */
    platform_load_resource(RES_STATIC_OVERLAY);
    platform_load_resource(RES_6AM);
    platform_load_resource(RES_GAME_OVER);
    platform_load_resource(RES_PAYCHECK);

    /* Load jumpscare resources */
    for (i = 0; i < ANIM_COUNT; i++) {
        platform_load_resource(RES_JUMPSCARE_BASE + i);
    }
}

void resource_load_menu(void)
{
    platform_load_resource(RES_MENU_BACKGROUND);
    platform_load_resource(RES_MENU_TITLE);
    platform_load_resource(RES_MENU_NEWSPAPER);
    platform_load_resource(RES_MENU_STAR);
    platform_load_resource(RES_STATIC_OVERLAY);
}

void resource_unload_gameplay(void)
{
    int i;

    platform_unload_resource(RES_OFFICE_DARK);
    platform_unload_resource(RES_OFFICE_FLASHLIGHT);
    platform_unload_resource(RES_OFFICE_LEFT_LIGHT);
    platform_unload_resource(RES_OFFICE_RIGHT_LIGHT);
    platform_unload_resource(RES_OFFICE_MASK);

    platform_unload_resource(RES_CAMERA_BORDER);
    platform_unload_resource(RES_CAMERA_MAP);
    platform_unload_resource(RES_CAMERA_STATIC);
    platform_unload_resource(RES_CAMERA_RECORDING);

    for (i = 0; i < CAM_COUNT; i++) {
        platform_unload_resource(RES_CAM_FEED_BASE + i);
    }

    for (i = 0; i < ANIM_COUNT; i++) {
        platform_unload_resource(RES_JUMPSCARE_BASE + i);
    }

    platform_unload_resource(RES_STATIC_OVERLAY);
    platform_unload_resource(RES_6AM);
    platform_unload_resource(RES_GAME_OVER);
    platform_unload_resource(RES_PAYCHECK);
}

void resource_unload_menu(void)
{
    platform_unload_resource(RES_MENU_BACKGROUND);
    platform_unload_resource(RES_MENU_TITLE);
    platform_unload_resource(RES_MENU_NEWSPAPER);
    platform_unload_resource(RES_MENU_STAR);
}
