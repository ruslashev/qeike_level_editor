#include "driver.hh"
#include "imgui.hh"
#include "utils.hh"
#define GLEW_STATIC
#include <GL/glew.h>
#include "imgui/imgui.h"

static SDL_Window *window;
static SDL_GLContext glcontext;
int pre_lock_mouse_x, pre_lock_mouse_y;

void driver_init() {
  assertf(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0
      , "failed to init sdl: %s", SDL_GetError());
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  window = SDL_CreateWindow("Qeike level editor"
      , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720
      , SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  glcontext = SDL_GL_CreateContext(window);
  pre_lock_mouse_x = 1280 / 2;
  pre_lock_mouse_y = 720 / 2;

  if (SDL_GL_SetSwapInterval(0) == -1)
    warning("failed to set vsync: %s", SDL_GetError());

  GLenum err = glewInit();
  assertf(err == GLEW_OK, "failed to initialze glew: %s",
      glewGetErrorString(err));

  imgui_init(window);

  glClearColor(0.07f, 0.07f, 0.07f, 0.07f);
}

static double get_time_in_seconds() {
  return SDL_GetTicks() / 1000.;
}

void driver_main_loop(void (*frame_cb)(void)
    , void (*update_cb)(double, double)
    , void (*key_event_cb)(char, bool)
    , void (*mouse_motion_event_cb)(float, float, int, int)
    , void (*mouse_button_event_cb)(int, bool, int, int)
    ) {
  bool done = false;
  double t = 0, dt = 1. / 60., current_time = get_time_in_seconds()
    , accumulator = 0;

  while (!done) {
    double real_time = get_time_in_seconds()
      , elapsed = real_time - current_time;
    current_time = real_time;
    accumulator += elapsed;

    while (accumulator >= dt) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        imgui_process_event(&event);
        if (event.type == SDL_QUIT)
          done = true;
        else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            && event.key.repeat == 0) {
          char key_info;
          switch (event.key.keysym.sym) {
            case SDLK_a:   key_info = 'a'; break;
            case SDLK_c:   key_info = 'c'; break;
            case SDLK_d:   key_info = 'd'; break;
            case SDLK_f:   key_info = 'f'; break;
            case SDLK_s:   key_info = 's'; break;
            case SDLK_w:   key_info = 'w'; break;
            case SDLK_x:   key_info = 'x'; break;
            case SDLK_z:   key_info = 'z'; break;
            case SDLK_TAB: key_info = '\t'; break;
            default:       key_info = 0;
          }
          if (key_info != 0)
            key_event_cb(key_info, event.type == SDL_KEYDOWN);
        } else if (event.type == SDL_MOUSEMOTION)
          mouse_motion_event_cb(static_cast<float>(event.motion.xrel)
              , static_cast<float>(event.motion.yrel), event.motion.x
              , event.motion.y);
        else if (event.type == SDL_MOUSEBUTTONDOWN
            || event.type == SDL_MOUSEBUTTONUP) {
          int button;
          switch (event.button.button) {
            case SDL_BUTTON_LEFT:   button = 1; break;
            case SDL_BUTTON_MIDDLE: button = 2; break;
            case SDL_BUTTON_RIGHT:  button = 3; break;
            case SDL_BUTTON_X1:     button = 4; break;
            case SDL_BUTTON_X2:     button = 5; break;
            default:                button = -1;
          }
          mouse_button_event_cb(button, event.type == SDL_MOUSEBUTTONDOWN
              , event.motion.x, event.motion.y);
        }
      }

      update_cb(dt, t);

      t += dt;
      accumulator -= dt;
    }

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x
        , (int)ImGui::GetIO().DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    imgui_new_frame(window);

    frame_cb();

    ImGui::Render();
    SDL_GL_SwapWindow(window);
  }
}

void driver_lock_mouse() {
  SDL_GetMouseState(&pre_lock_mouse_x, &pre_lock_mouse_y);
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void driver_unlock_mouse() {
  SDL_SetRelativeMouseMode(SDL_FALSE);
  SDL_WarpMouseInWindow(window, pre_lock_mouse_x, pre_lock_mouse_y);
}

void driver_destroy() {
  imgui_destroy();
  SDL_GL_DeleteContext(glcontext);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

