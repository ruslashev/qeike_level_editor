#pragma once

void driver_init();
void driver_main_loop(void (*frame_cb)(void)
    , void (*update_cb)(double, double)
    , void (*key_event_cb)(char, bool)
    , void (*mouse_motion_event_cb)(float, float, int, int)
    , void (*mouse_button_event_cb)(int, bool));
void driver_lock_mouse();
void driver_unlock_mouse();
void driver_destroy();

