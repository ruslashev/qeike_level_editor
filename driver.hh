#pragma once

void driver_init();
void driver_main_loop(void (*frame_cb)(void));
void driver_destroy();

