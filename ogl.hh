#pragma once

#include "utils.hh"
#include <GL/glew.h>
#include <vector>
#include <string>
#include <cstring>
#include <glm/mat4x4.hpp>

void gl_error_description(GLenum err);

#define gl_check_errors() do { \
  GLenum err = glGetError(); \
  while (err != GL_NO_ERROR) { \
    printf("glError %d at %s:%d\n", err, __FILE__, __LINE__); \
    gl_error_description(err); \
    err = glGetError(); \
  } \
} while (0)

class ogl_buffer {
protected:
  GLuint _id;
  GLenum _type;
public:
  ogl_buffer(GLenum n_type);
  ~ogl_buffer();
  void bind() const;
  void unbind() const;
};

class array_buffer : public ogl_buffer {
public:
  array_buffer();
  void upload(const std::vector<GLfloat> &data) const;
  void upload(const std::vector<glm::vec3> &data) const;
  void upload(int size, const void *data) const;
};

class element_array_buffer : public ogl_buffer {
public:
  element_array_buffer();
  void upload(const std::vector<GLushort> &data) const;
  void upload(const std::vector<GLuint> &data) const;
  void upload(int size, const void *data) const;
};

struct shader {
  GLuint type;
  GLuint id;
  shader(std::string source, GLuint ntype);
  ~shader();
};

class shader_program {
  void _create_from_two_shaders(const shader &vert, const shader &frag);
public:
  GLuint id;
  shader_program(const shader &vert, const shader &frag);
  shader_program(std::string vert_src, std::string frag_src);
  ~shader_program();
  GLint bind_attrib(const char *name);
  GLint bind_attrib_preserve_prog(const char *name);
  GLint bind_uniform(const char *name);
  GLint bind_uniform_preserve_prog(const char *name);
  void use_this_prog();
  void dont_use_this_prog();
};

struct vertex_array_object {
  GLuint id;
  vertex_array_object();
  ~vertex_array_object();
  void bind() const;
  void unbind() const;
};

class axis_drawer {
  vertex_array_object _vao;
  array_buffer _vbo;
  element_array_buffer _ebo;
  shader_program _sp;
  GLint _vertex_pos_attr, _vertex_color_attr, _mvp_mat_unif;
public:
  axis_drawer();
  void draw(const glm::mat4 &mvp);
};

class grid_drawer {
  vertex_array_object _vao;
  array_buffer _vbo;
  element_array_buffer _ebo;
  shader_program _sp;
  GLint _vertex_pos_attr, _mvp_mat_unif;
  int _n_elements;
public:
  grid_drawer(int size, float density);
  void draw(const glm::mat4 &mvp);
};

