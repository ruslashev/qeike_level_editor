#pragma once

#include "ogl.hh"
#include <glm/vec3.hpp>

class brush {
  vertex_array_object _vao;
  array_buffer _vbo;
  element_array_buffer _ebo;
  shader_program _sp;
  GLint _vertex_pos_attr, _vertex_normal_attr, _model_mat_unif, _view_mat_unif
    , _projection_mat_unif, _object_color_unif, _view_pos_unif, _light_pos_unif;
  // glm::vec3 _control_points[8];
public:
  brush();
  void draw(const glm::vec3 &position, const glm::mat4 &model
    , const glm::mat4 &view, const glm::mat4 &projection) const;
};

