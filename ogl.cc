#include "ogl.hh"
#include "shaders.hh"
#include <glm/gtc/type_ptr.hpp>

void gl_error_description(GLenum err) {
  switch (err) {
    case GL_INVALID_ENUM:
      puts("GL_INVALID_ENUM: An unacceptable value is specified for an\n"
          "enumerated argument. The offending command is ignored and has\n"
          "no other side effect than to set the error flag.");
      break;
    case GL_INVALID_VALUE:
      puts("GL_INVALID_VALUE: A numeric argument is out of range. The\n"
          "offending command is ignored and has no other side effect than\n"
          "to set the error flag.");
      break;
    case GL_INVALID_OPERATION:
      puts("GL_INVALID_OPERATION: The specified operation is not allowed in\n"
          "the current state. The offending command is ignored and has no\n"
          "other side effect than to set the error flag.");
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      puts("GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not\n"
          "complete. The offending command is ignored and has no other side\n"
          "effect than to set the error flag.");
      break;
    case GL_OUT_OF_MEMORY:
      puts("GL_OUT_OF_MEMORY: There is not enough memory left to execute the\n"
          "command. The state of the GL is undefined, except for the state\n"
          "of the error flags, after this error is recorded.");
      break;
    case GL_STACK_UNDERFLOW:
      puts("GL_STACK_UNDERFLOW: An attempt has been made to perform an\n"
          "operation that would cause an internal stack to underflow.");
      break;
    case GL_STACK_OVERFLOW:
      puts("GL_STACK_OVERFLOW: An attempt has been made to perform an\n"
          "operation that would cause an internal stack to overflow.");
      break;
    default:
      break;
  }
}

ogl_buffer::ogl_buffer(GLenum n_type) : _type(n_type) {
  glGenBuffers(1, &_id);
}

ogl_buffer::~ogl_buffer() {
  glDeleteBuffers(1, &_id);
}

void ogl_buffer::bind() const {
  glBindBuffer(_type, _id);
}

void ogl_buffer::unbind() const {
  glBindBuffer(_type, 0);
}

array_buffer::array_buffer() : ogl_buffer(GL_ARRAY_BUFFER) {
}

void array_buffer::upload(const std::vector<GLfloat> &data) const {
  glBufferData(GL_ARRAY_BUFFER
      , static_cast<GLsizeiptr>(data.size() * sizeof(data[0])), data.data()
      , GL_STATIC_DRAW);
}

void array_buffer::upload(const std::vector<glm::vec3> &data) const {
  glBufferData(GL_ARRAY_BUFFER
      , static_cast<GLsizeiptr>(data.size() * sizeof(data[0])), data.data()
      , GL_STATIC_DRAW);
}

void array_buffer::upload(int size, const void *data) const {
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

element_array_buffer::element_array_buffer()
  : ogl_buffer(GL_ELEMENT_ARRAY_BUFFER) {
}

void element_array_buffer::upload(const std::vector<GLushort> &data) const {
  glBufferData(GL_ELEMENT_ARRAY_BUFFER
      , static_cast<GLsizeiptr>(data.size() * sizeof(data[0])), data.data()
      , GL_STATIC_DRAW);
}

void element_array_buffer::upload(const std::vector<GLuint> &data) const {
  glBufferData(GL_ELEMENT_ARRAY_BUFFER
      , static_cast<GLsizeiptr>(data.size() * sizeof(data[0])), data.data()
      , GL_STATIC_DRAW);
}

void element_array_buffer::upload(int size, const void *data) const {
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

static std::string get_ogl_shader_err(GLint loglen
    , void (*ogl_errmsg_func)(GLuint, GLsizei, GLsizei*, GLchar*)
    , GLuint id) {
  char msg[loglen + 1];
  ogl_errmsg_func(id, loglen, nullptr, msg);
  msg[loglen] = 0;
  std::string msgstr(msg);
  msgstr.pop_back(); // strip trailing newline
  /*
  // indent every line
  int indent = 3;
  msgstr.insert(msgstr.begin(), indent, ' ');
  for (size_t i = 0; i < msgstr.size(); i++) {
    if (msgstr[i] != '\n')
      continue;
    msgstr.insert(i, indent, ' ');
  }
  */
  return msgstr;
}

shader::shader(std::string source, GLuint ntype) : type(ntype) {
  id = glCreateShader(type);
  const char *csrc = source.c_str();
  glShaderSource(id, 1, &csrc, NULL);
  glCompileShader(id);
  GLint loglen;
  glGetShaderiv(id, GL_INFO_LOG_LENGTH, &loglen);
  if (loglen != 0) {
    std::string msg = get_ogl_shader_err(loglen, glGetShaderInfoLog, id);
    GLint compilesucc;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compilesucc);
    if (compilesucc != GL_TRUE)
      die("failed to compile %s shader:\n%s"
          , type == GL_VERTEX_SHADER ? "vertex" : "fragment"
          , msg.c_str());
    else if (0)
      printf("### %s shader diagnostic message:\n%s\n###\n"
          , type == GL_VERTEX_SHADER ? "vertex" : "fragment"
          , msg.c_str());
  }
}

shader::~shader() {
  glDeleteShader(id);
}

void shader_program::_create_from_two_shaders(const shader &vert
    , const shader &frag) {
  assertf(vert.type == GL_VERTEX_SHADER && frag.type == GL_FRAGMENT_SHADER
      , "order of shaders in shader_program's constructor is reversed");
  id = glCreateProgram();
  glAttachShader(id, vert.id);
  glAttachShader(id, frag.id);
  glLinkProgram(id);
  GLint loglen;
  glGetProgramiv(id, GL_INFO_LOG_LENGTH, &loglen);
  if (loglen != 0) {
    std::string msg = get_ogl_shader_err(loglen, glGetProgramInfoLog, id);
    GLint link_status;
    glGetProgramiv(id, GL_LINK_STATUS, &link_status);
    if (link_status != GL_TRUE) {
      glDetachShader(id, vert.id);
      glDetachShader(id, frag.id);
      die("failed to link a program:\n%s", msg.c_str());
    } else
      printf("### shader program diagnostic message:\n%s\n"
          "### diagnostic message end\n", msg.c_str());
  }
}

shader_program::shader_program(const shader &vert, const shader &frag) {
  _create_from_two_shaders(vert, frag);
}

shader_program::shader_program(std::string vert_src, std::string frag_src) {
  shader vert(vert_src, GL_VERTEX_SHADER), frag(frag_src, GL_FRAGMENT_SHADER);
  _create_from_two_shaders(vert, frag);
}

shader_program::~shader_program() {
  glDeleteProgram(id);
}

GLint shader_program::bind_attrib(const char *name) {
  GLint attr = glGetAttribLocation(id, name);
  assertf(glGetError() == GL_NO_ERROR, "failed to bind attribute %s", name);
  if (attr == -1)
    warning("failed to bind attribute %s", name);
  return attr;
}

GLint shader_program::bind_attrib_preserve_prog(const char *name) {
  GLint prev_active_prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prev_active_prog);
  use_this_prog();
  GLint attr = glGetAttribLocation(id, name);
  assertf(glGetError() == GL_NO_ERROR, "failed to bind attribute %s", name);
  if (attr == -1)
    warning("failed to bind attribute %s", name);
  glUseProgram(static_cast<GLuint>(prev_active_prog));
  return attr;
}

GLint shader_program::bind_uniform(const char *name) {
  GLint unif = glGetUniformLocation(id, name);
  assertf(glGetError() == GL_NO_ERROR, "failed to bind uniform %s", name);
  if (unif == -1)
    warning("failed to bind uniform %s", name);
  return unif;
}

GLint shader_program::bind_uniform_preserve_prog(const char *name) {
  GLint prev_active_prog;
  glGetIntegerv(GL_CURRENT_PROGRAM, &prev_active_prog);
  use_this_prog();
  GLint unif = glGetUniformLocation(id, name);
  assertf(glGetError() == GL_NO_ERROR, "failed to bind uniform %s", name);
  if (unif == -1)
    warning("failed to bind uniform %s", name);
  glUseProgram(static_cast<GLuint>(prev_active_prog));
  return unif;
}

void shader_program::use_this_prog() {
  glUseProgram(id);
}

void shader_program::dont_use_this_prog() {
  glUseProgram(0);
}

vertex_array_object::vertex_array_object() {
  glGenVertexArrays(1, &id);
}

vertex_array_object::~vertex_array_object() {
  glDeleteVertexArrays(1, &id);
}

void vertex_array_object::bind() const {
  glBindVertexArray(id);
}

void vertex_array_object::unbind() const {
  glBindVertexArray(0);
}

axis_drawer::axis_drawer()
  : _sp(shaders::colored_mesh_vert, shaders::colored_mesh_frag)
  , _vertex_pos_attr(_sp.bind_attrib("vertex_pos"))
  , _vertex_color_attr(_sp.bind_attrib("vertex_color"))
  , _mvp_mat_unif(_sp.bind_uniform("mvp")) {
  _vao.bind();
  _vbo.bind();
  _ebo.bind();
  // vec3 vertex position and vec3 color
  const std::vector<float> verts = {
    0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1,
    1, 0, 0, 1, 0, 0,
    0, 1, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 1
  };
  _vbo.upload(verts);
  const std::vector<GLushort> elements = {
    0, 3,
    1, 4,
    2, 5
  };
  _ebo.upload(elements);
  glEnableVertexAttribArray(_vertex_pos_attr);
  glVertexAttribPointer(_vertex_pos_attr, 3, GL_FLOAT, GL_FALSE
      , 6 * sizeof(float), 0);
  glEnableVertexAttribArray(_vertex_color_attr);
  glVertexAttribPointer(_vertex_color_attr, 3, GL_FLOAT, GL_FALSE
      , 6 * sizeof(float), (void*)(3 * sizeof(float)));
  _vao.unbind();
  _vbo.unbind();
  _ebo.unbind();
  glDisableVertexAttribArray(_vertex_pos_attr);
  glDisableVertexAttribArray(_vertex_color_attr);
}

void axis_drawer::draw(const glm::mat4 &mvp) {
  _vao.bind();
  _sp.use_this_prog();
  glUniformMatrix4fv(_mvp_mat_unif, 1, GL_FALSE, glm::value_ptr(mvp));
  glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, 0);
  _vao.unbind();
}

grid_drawer::grid_drawer(int size, float density)
  : _sp(shaders::simple_vert, shaders::simple_frag)
  , _vertex_pos_attr(_sp.bind_attrib("vertex_pos"))
  , _mvp_mat_unif(_sp.bind_uniform("mvp")) {
  _vao.bind();
  _vbo.bind();
  _ebo.bind();
  std::vector<float> verts;
  for (int z = -size; z <= size; ++z)
    for (int x = -size; x <= size; ++x) {
      verts.push_back(x * density);
      verts.push_back(0 * density);
      verts.push_back(z * density);
    }
  _vbo.upload(verts);
  const int width = size * 2 + 1, height = width;
  std::vector<GLushort> elements;
  for (int i = 0; i < width * height; ++i) {
    if ((i % width) != width - 1) {
      elements.push_back(i);
      elements.push_back(i + 1);
    }
    if (i < width * height - width) {
      elements.push_back(i);
      elements.push_back(i + width);
    }
  }
  _n_elements = elements.size();
  _ebo.upload(elements);
  glEnableVertexAttribArray(_vertex_pos_attr);
  glVertexAttribPointer(_vertex_pos_attr, 3, GL_FLOAT, GL_FALSE
      , 3 * sizeof(float), 0);
  _vao.unbind();
  _vbo.unbind();
  _ebo.unbind();
  glDisableVertexAttribArray(_vertex_pos_attr);
}

void grid_drawer::draw(const glm::mat4 &mvp) {
  _vao.bind();
  _sp.use_this_prog();
  glUniformMatrix4fv(_mvp_mat_unif, 1, GL_FALSE, glm::value_ptr(mvp));
  glDrawElements(GL_LINES, _n_elements, GL_UNSIGNED_SHORT, 0);
  _vao.unbind();
}

