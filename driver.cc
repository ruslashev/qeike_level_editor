#include "driver.hh"
#include "imgui.hh"
#include "utils.hh"
#include <GL/glew.h>
#include "imgui/imgui.h"

SDL_Window *g_window;
SDL_GLContext glcontext;

void driver_init() {
  assertf(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == 0
      , "failed to init sdl: %s", SDL_GetError());
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  g_window = SDL_CreateWindow("Qeike level editor"
      , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720
      , SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  glcontext = SDL_GL_CreateContext(g_window);

  if (SDL_GL_SetSwapInterval(0) == -1)
    warning("failed to set vsync: %s", SDL_GetError());

  GLenum err = glewInit();
  assertf(err == GLEW_OK, "failed to initialze glew: %s",
      glewGetErrorString(err));

  imgui_init(g_window);

  glClearColor(0.07f, 0.07f, 0.07f, 0.07f);
}

void driver_main_loop(void (*frame_cb)(void)) {
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      imgui_process_event(&event);
      if (event.type == SDL_QUIT)
        done = true;
    }

    imgui_new_frame(g_window);

    frame_cb();

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x
        , (int)ImGui::GetIO().DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    SDL_GL_SwapWindow(g_window);
  }
}

void driver_destroy() {
  imgui_destroy();
  SDL_GL_DeleteContext(glcontext);
  SDL_DestroyWindow(g_window);
  SDL_Quit();
}

