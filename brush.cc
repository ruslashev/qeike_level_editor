#include "brush.hh"
#include "shaders.hh"
#include <glm/gtc/type_ptr.hpp>

brush::brush()
  : _sp(shaders::simple_vert, shaders::simple_frag)
  , _vertex_pos_attr(_sp.bind_attrib("vertex_pos"))
  , _mvp_mat_unif(_sp.bind_uniform("mvp"))
  , _control_points(
      { glm::vec3(-0.5, -0.5,  0.5)
      , glm::vec3( 0.5, -0.5,  0.5)
      , glm::vec3( 0.5,  0.5,  0.5)
      , glm::vec3(-0.5,  0.5,  0.5)
      , glm::vec3(-0.5, -0.5, -0.5)
      , glm::vec3( 0.5, -0.5, -0.5)
      , glm::vec3( 0.5,  0.5, -0.5)
      , glm::vec3(-0.5,  0.5, -0.5)
      }) {
  _vao.bind();
  _vbo.bind();
  _ebo.bind();
  _vbo.upload(8 * sizeof(glm::vec3), _control_points);
  const std::vector<GLushort> elements = {
    0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3,
  };
  _ebo.upload(elements);
  glEnableVertexAttribArray(_vertex_pos_attr);
  glVertexAttribPointer(_vertex_pos_attr, 3, GL_FLOAT, GL_FALSE
      , 3 * sizeof(float), 0);
  _vao.unbind();
  _vbo.unbind();
  _ebo.unbind();
  glDisableVertexAttribArray(_vertex_pos_attr);
}

void brush::draw(const glm::mat4 &mvp) const {
  _vao.bind();
  _sp.use_this_prog();
  glUniformMatrix4fv(_mvp_mat_unif, 1, GL_FALSE, glm::value_ptr(mvp));
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
  _vao.unbind();
}

