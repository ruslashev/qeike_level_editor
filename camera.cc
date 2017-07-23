#include "camera.hh"
#include "utils.hh"

camera::camera()
  : pitch(0)
  , yaw(270)
  , roll(0)
  , pos(glm::vec3(0, 1, 2))
  , vel(glm::vec3(0, 0, 0)) {
}

void camera::update_view_angles(float xrel, float yrel) {
  yaw += xrel * 3.f * 0.022f;
  pitch -= yrel * 3.f * 0.022f;

  if (yaw >= 360.f)
    yaw -= 360.f;
  if (yaw < 0.f)
    yaw += 360.f;

  if (pitch >= 89.99f)
    pitch = 89.99f;
  if (pitch <= -89.99f)
    pitch = -89.99f;
}

void camera::update_position(double dt, int move, int strafe) {
  const float pitch_rad = glm::radians(pitch), yaw_rad = glm::radians(yaw)
    , perp_yaw = yaw_rad + static_cast<float>(M_PI_2);
  float dist_move = 1.f * static_cast<float>(move * dt)
    , dist_strafe = 1.f * static_cast<float>(strafe * dt);
  if (move == strafe) {
    dist_move /= sqrtf(2);
    dist_strafe /= sqrtf(2);
  }
  pos.x += cosf(yaw_rad) * cosf(pitch_rad) * dist_move
    + cosf(perp_yaw) * dist_strafe;
  pos.y += sinf(pitch_rad) * dist_move;
  pos.z += sinf(yaw_rad) * cosf(pitch_rad) * dist_move
    + sinf(perp_yaw) * dist_strafe;
}

glm::vec3 camera::compute_view_dir() const {
  // TODO called twice per frame, cache this?
  const float pitch_rad = glm::radians(pitch), yaw_rad = glm::radians(yaw);
  return glm::vec3(cos(yaw_rad) * cos(pitch_rad), sin(pitch_rad)
      , sin(yaw_rad) * cos(pitch_rad));
}

glm::mat4 camera::compute_view_mat() const {
  glm::vec3 look_at = pos + compute_view_dir();
  return glm::lookAt(pos, look_at, glm::vec3(0, 1, 0));
}

