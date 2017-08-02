#include "brush.hh"
#include "camera.hh"
#include "driver.hh"
#include "imgui.hh"
#include "imgui/imgui.h"
#include "ogl.hh"
#include "utils.hh"
#include <memory>
#include <GL/glew.h>

static const double fov = 45, dt = 1. / 60.;
static int move = 0, strafe = 0;
static camera *cam;
static bool show_test_window = false;
static axis_drawer *ad;
static grid_drawer *gd;
static enum class editor_mode_t {
  editing, lookaround
} editor_mode = editor_mode_t::editing;
static bool dragging = false;
static float dragging_x = 0, dragging_y = 0;
std::vector<std::unique_ptr<brush>> brushes;

static void load() {
  cam = new camera;
  ad = new axis_drawer;
  gd = new grid_drawer(10, 1.f);
}

static void key_event(char key, bool down) {
  static bool forward = false, backward = false, left = false, right = false;
  switch (key) {
    case 'w': forward  = down; break;
    case 's': backward = down; break;
    case 'd': right    = down; break;
    case 'a': left     = down; break;
    case '\t':
      if (down) {
        if (editor_mode == editor_mode_t::editing) {
          driver_lock_mouse(); // TODO bad
          editor_mode = editor_mode_t::lookaround;
        } else if (editor_mode == editor_mode_t::lookaround) {
          driver_unlock_mouse();
          editor_mode = editor_mode_t::editing;
        }
      }
      break;
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
  if (editor_mode == editor_mode_t::lookaround)
    cam->update_view_angles(xrel, yrel);
  if (dragging) {
    dragging_x += xrel;
    dragging_y += yrel;
  }
}

static void mouse_button_event(int button, bool down, int x, int y) {
  if (button == 1)
    dragging = down;
}

static void update(double dt, double t) {
  cam->update_position(dt, move, strafe);
}

static void frame() {
  if (ImGui::BeginMainMenuBar()) {
    { // mode
      if (editor_mode == editor_mode_t::editing)
        ImGui::TextColored(ImVec4(0.886f, 0.717f, 0.898f, 1.f), "Editing");
      else if (editor_mode == editor_mode_t::lookaround)
        ImGui::TextColored(ImVec4(0.886f, 0.898f, 0.717f, 1.f), "Lookaround");
    }
    ImGui::SameLine();
    { // new brush button
      if (ImGui::SmallButton("New brush"))
        brushes.push_back(std::unique_ptr<brush>(new brush));
    }
    ImGui::SameLine();
    { // temp. show test window button
      if (ImGui::SmallButton("Show test window"))
        show_test_window = true;
    }
    ImGui::EndMainMenuBar();
  }

  if (show_test_window) {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow(&show_test_window);
  }

  float screen_aspect_ratio = (float)ImGui::GetIO().DisplaySize.x
    / (float)ImGui::GetIO().DisplaySize.y;
  glm::mat4 projection = glm::perspective((float)glm::radians(fov)
      , screen_aspect_ratio, 0.1f, 300.f), view = cam->compute_view_mat()
    , model = glm::rotate(glm::rotate(glm::mat4(), glm::radians(dragging_y)
          , glm::vec3(1, 0, 0)), glm::radians(dragging_x), glm::vec3(0, 1, 0))
    , mvp = projection * view * model;

  ad->draw(mvp);
  gd->draw(mvp);

  for (const std::unique_ptr<brush> &b : brushes)
    b->draw(cam->pos, model, view, projection);
}

static void cleanup() {
  delete cam;
  delete ad;
  delete gd;
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

