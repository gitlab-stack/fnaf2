/*
 * camera.c - Camera monitoring system implementation
 *
 * Five Nights at Freddy's 2 - C Port
 */

#include "camera.h"
#include "platform.h"

#define CAMERA_STATIC_DURATION_MS   300     /* Static on camera switch */

void camera_init(CameraSystem *cam)
{
    FNAF2_MEMSET(cam, 0, sizeof(CameraSystem));
    cam->current_cam = CAM_SHOW_STAGE;
    cam->previous_cam = CAM_SHOW_STAGE;
    cam->static_duration = CAMERA_STATIC_DURATION_MS;
}

void camera_reset(CameraSystem *cam)
{
    cam->is_open = false;
    cam->current_cam = CAM_SHOW_STAGE;
    cam->previous_cam = CAM_SHOW_STAGE;
    cam->open_timer = 0;
    cam->static_timer = 0;
    cam->static_active = false;
}

void camera_toggle(CameraSystem *cam)
{
    if (cam->is_open) {
        camera_close(cam);
    } else {
        camera_open(cam);
    }
}

void camera_open(CameraSystem *cam)
{
    if (!cam->is_open) {
        cam->is_open = true;
        cam->open_timer = 0;
        cam->static_active = true;
        cam->static_timer = 0;
        platform_play_sound(SND_CAMERA_OPEN, false);
    }
}

void camera_close(CameraSystem *cam)
{
    if (cam->is_open) {
        cam->is_open = false;
        cam->open_timer = 0;
        platform_play_sound(SND_CAMERA_CLOSE, false);
    }
}

void camera_switch(CameraSystem *cam, int camera_id)
{
    if (camera_id < 0 || camera_id >= CAM_COUNT) return;
    if (camera_id == cam->current_cam) return;

    cam->previous_cam = cam->current_cam;
    cam->current_cam = camera_id;
    cam->static_active = true;
    cam->static_timer = 0;
    platform_play_sound(SND_CAMERA_SWITCH, false);
}

void camera_next(CameraSystem *cam)
{
    int next = cam->current_cam + 1;
    if (next >= CAM_COUNT) next = 0;
    camera_switch(cam, next);
}

void camera_prev(CameraSystem *cam)
{
    int prev = cam->current_cam - 1;
    if (prev < 0) prev = CAM_COUNT - 1;
    camera_switch(cam, prev);
}

void camera_update(CameraSystem *cam, uint32_t delta_ms)
{
    if (cam->is_open) {
        cam->open_timer += delta_ms;
    }

    /* Update static effect */
    if (cam->static_active) {
        cam->static_timer += delta_ms;
        if (cam->static_timer >= cam->static_duration) {
            cam->static_active = false;
            cam->static_timer = 0;
        }
    }
}

bool camera_viewing(const CameraSystem *cam, int camera_id)
{
    return cam->is_open && cam->current_cam == camera_id;
}

bool camera_on_prize_corner(const CameraSystem *cam)
{
    return cam->is_open && cam->current_cam == CAM_PRIZE_CORNER;
}
