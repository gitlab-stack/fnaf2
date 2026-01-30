/*
 * resource.h - Resource/asset management
 *
 * Five Nights at Freddy's 2 - C Port
 *
 * Manages loading and unloading of game resources.
 * The actual loading is done by the platform layer.
 */

#ifndef FNAF2_RESOURCE_H
#define FNAF2_RESOURCE_H

#include "types.h"
#include "game_defs.h"

/* Resource path mapping - maps ResourceID to file path */
typedef struct {
    int         id;
    const char *path;
} ResourceMapping;

/* Get the file path for a resource ID.
 * Returns NULL if the resource has no mapping. */
const char *resource_get_path(int resource_id);

/* Load resources needed for gameplay */
void resource_load_gameplay(void);

/* Load resources needed for menu */
void resource_load_menu(void);

/* Unload gameplay resources */
void resource_unload_gameplay(void);

/* Unload menu resources */
void resource_unload_menu(void);

#endif /* FNAF2_RESOURCE_H */
