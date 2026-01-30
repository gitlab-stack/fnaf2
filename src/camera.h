/*
 * camera.h - Camera monitoring system
 *
 * Five Nights at Freddy's 2 - C Port
 */

#ifndef FNAF2_CAMERA_H
#define FNAF2_CAMERA_H

#include "types.h"
#include "game_defs.h"

typedef struct {
    bool        is_open;            /* Camera panel is raised */
    int         current_cam;        /* Currently viewed camera (0-11) */
    int         previous_cam;       /* Previously viewed camera */
    uint32_t    open_timer;         /* How long camera has been open (ms) */
    uint32_t    static_timer;       /* Static effect timer on camera switch */
    bool        static_active;      /* Currently showing static (switching) */
    uint32_t    static_duration;    /* Duration of static on switch */
} CameraSystem;

/* Initialize camera system */
void camera_init(CameraSystem *cam);

/* Reset for new night */
void camera_reset(CameraSystem *cam);

/* Toggle camera panel open/closed */
void camera_toggle(CameraSystem *cam);

/* Open the camera panel */
void camera_open(CameraSystem *cam);

/* Close the camera panel */
void camera_close(CameraSystem *cam);

/* Switch to a specific camera */
void camera_switch(CameraSystem *cam, int camera_id);

/* Switch to next/previous camera */
void camera_next(CameraSystem *cam);
void camera_prev(CameraSystem *cam);

/* Update camera system (handles static timer, etc.) */
void camera_update(CameraSystem *cam, uint32_t delta_ms);

/* Check if currently viewing a specific camera */
bool camera_viewing(const CameraSystem *cam, int camera_id);

/* Check if camera is on prize corner (for music box) */
bool camera_on_prize_corner(const CameraSystem *cam);

#endif /* FNAF2_CAMERA_H */
