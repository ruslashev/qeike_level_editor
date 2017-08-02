#include "brush.hh"
#include "shaders.hh"
#include <glm/gtc/type_ptr.hpp>

struct vertex {
  glm::vec3 p, n;
};

brush::brush()
  : _sp(shaders::light_vert, shaders::light_frag)
  , _vertex_pos_attr(_sp.bind_attrib("vertex_pos"))
  , _vertex_normal_attr(_sp.bind_attrib("vertex_normal"))
  , _model_mat_unif(_sp.bind_uniform("model"))
  , _view_mat_unif(_sp.bind_uniform("view"))
  , _projection_mat_unif(_sp.bind_uniform("projection"))
  , _object_color_unif(_sp.bind_uniform("object_color"))
  , _view_pos_unif(_sp.bind_uniform("view_pos"))
  , _light_pos_unif(_sp.bind_uniform("light_pos")) {

  _vao.bind();
  _sp.use_this_prog();

  _vbo.bind();
  const float n = 1.f / sqrtf(3.f);
  vertex _control_points[8] =
      { glm::vec3(-.5f, -.5f,  .5f), glm::vec3(-n, -n,  n)
      , glm::vec3( .5f, -.5f,  .5f), glm::vec3( n, -n,  n)
      , glm::vec3( .5f,  .5f,  .5f), glm::vec3( n,  n,  n)
      , glm::vec3(-.5f,  .5f,  .5f), glm::vec3(-n,  n,  n)
      , glm::vec3(-.5f, -.5f, -.5f), glm::vec3(-n, -n, -n)
      , glm::vec3( .5f, -.5f, -.5f), glm::vec3( n, -n, -n)
      , glm::vec3( .5f,  .5f, -.5f), glm::vec3( n,  n, -n)
      , glm::vec3(-.5f,  .5f, -.5f), glm::vec3(-n,  n, -n)
      };
  _vbo.upload(8 * 2 * sizeof(glm::vec3), _control_points);

  _ebo.bind();
  const std::vector<GLushort> elements = {
    0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3,
  };
  _ebo.upload(elements);

  glEnableVertexAttribArray(_vertex_pos_attr);
  glVertexAttribPointer(_vertex_pos_attr, 3, GL_FLOAT, GL_FALSE
      , 6 * sizeof(float), 0);

  glEnableVertexAttribArray(_vertex_normal_attr);
  glVertexAttribPointer(_vertex_pos_attr, 3, GL_FLOAT, GL_FALSE
      , 6 * sizeof(float), (void*)offsetof(vertex, n));

  _vao.unbind();
  _vbo.unbind();
  _ebo.unbind();
  glDisableVertexAttribArray(_vertex_pos_attr);
  glDisableVertexAttribArray(_vertex_normal_attr);
}

void brush::draw(const glm::vec3 &position, const glm::mat4 &model
    , const glm::mat4 &view, const glm::mat4 &projection) const {
  _vao.bind();
  _sp.use_this_prog();
  glUniform3f(_object_color_unif, 1, 0, 1);
  glUniform3f(_view_pos_unif, position.x, position.y, position.z);
  glUniform3f(_light_pos_unif, 0, 2, 0);
  glUniformMatrix4fv(_model_mat_unif, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(_view_mat_unif, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(_projection_mat_unif, 1, GL_FALSE, glm::value_ptr(projection));
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
  _vao.unbind();
}

