#include "camera.hh"
#include "driver.hh"
#include "imgui.hh"
#include "imgui/imgui.h"
#include "utils.hh"
#include "ogl.hh"
#include <GL/glew.h>

static const double fov = 60, dt = 1. / 60.;
static int move = 0, strafe = 0;
static camera *cam;
static bool show_test_window = true;
static axis_drawer *ad;

static void load() {
  cam = new camera;
  ad = new axis_drawer;
}

static void key_event(char key, bool down) {
  static bool forward = false, backward = false, left = false, right = false;
  switch (key) {
    case 'w': forward  = down; break;
    case 's': backward = down; break;
    case 'd': right    = down; break;
    case 'a': left     = down; break;
    default: break;
  }
  if (forward == backward)
    move = 0;
  else if (forward)
    move = 1;
  else if (backward)
    move = -1;
  if (right == left)
    strafe = 0;
  else if (right)
    strafe = 1;
  else if (left)
    strafe = -1;
}

static void mouse_motion_event(float xrel, float yrel, int x, int y) {
  // cam->update_view_angles(xrel, yrel);
}

static void mouse_button_event(int button, bool down) {
}

static void update(double dt, double t) {
  cam->update_position(dt, move, strafe);
}

static void frame() {
  static float f = 0.0f;
  ImGui::Text("Hello, world!");
  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
  if (ImGui::Button("Test Window")) show_test_window ^= 1;
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)"
      , (double)(1000.0f / ImGui::GetIO().Framerate)
      , (double)ImGui::GetIO().Framerate);

  if (show_test_window) {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow(&show_test_window);
  }

  float screen_aspect_ratio = (float)ImGui::GetIO().DisplaySize.x
    / (float)ImGui::GetIO().DisplaySize.y;
  glm::mat4 projection = glm::perspective((float)glm::radians(fov)
      , screen_aspect_ratio, 0.1f, 300.f), view = cam->compute_view_mat()
    , mvp = projection * view;

  ad->draw(mvp);
}

static void cleanup() {
  delete cam;
  delete ad;
}

int main() {
  driver_init();

  load();

  driver_main_loop(frame, update, key_event, mouse_motion_event
      , mouse_button_event);

  cleanup();

  driver_destroy();

  return 0;
}

