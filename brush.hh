#pragma once

#include "ogl.hh"
#include <glm/vec3.hpp>

class brush {
  vertex_array_object _vao;
  array_buffer _vbo;
  element_array_buffer _ebo;
  shader_program _sp;
  GLint _vertex_pos_attr, _mvp_mat_unif;
  glm::vec3 _control_points[8];
public:
  brush();
  void draw(const glm::mat4 &mvp) const;
};

