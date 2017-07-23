#include "imgui/imgui.h"
#include "imgui.hh"
#include "utils.hh"
#include "driver.hh"
#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL2/SDL.h>

bool show_test_window = true;
ImVec4 clear_color = ImColor(114, 144, 154);

void frame() {
  static float f = 0.0f;
  ImGui::Text("Hello, world!");
  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
  ImGui::ColorEdit3("clear color", (float*)&clear_color);
  if (ImGui::Button("Test Window")) show_test_window ^= 1;
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  if (show_test_window) {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
    ImGui::ShowTestWindow(&show_test_window);
  }
}

int main() {
  driver_init();

  driver_main_loop(frame);

  driver_destroy();

  return 0;
}

