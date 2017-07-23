#pragma once

#include <SDL2/SDL.h>

bool imgui_init(SDL_Window* window);
void imgui_destroy();
void imgui_new_frame(SDL_Window* window);
bool imgui_process_event(const SDL_Event* event);

