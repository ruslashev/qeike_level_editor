#pragma once

namespace shaders {

static const char *light_vert = _glsl(
  attribute vec3 vertex_pos;
  attribute vec3 vertex_normal;
  uniform mat4 model;
  uniform mat4 view;
  uniform mat4 projection;
  varying vec3 vertex_pos_f;
  varying vec3 vertex_normal_f;
  void main() {
    vertex_pos_f = vec3(model * vec4(vertex_pos, 1.0f));
    vertex_normal_f = normalize(mat3(transpose(inverse(model))) * vertex_normal);
    gl_Position = projection * view * model * vec4(vertex_pos, 1.0);
  }
);

static const char *light_frag = _glsl(
  uniform vec2 iResolution;
  uniform float iGlobalTime;
  uniform vec3 light_pos;
  uniform vec3 view_pos;
  uniform vec3 object_color;
  varying vec3 vertex_normal_f;
  varying vec3 vertex_pos_f;
  void main() {
    vec3 light_color = vec3(1, 1, 1);
    float ambient_strength = 0.1f;
    vec3 ambient = ambient_strength * light_color;

    vec3 light_dir = normalize(light_pos - vertex_pos_f);
    float diff = max(dot(vertex_normal_f, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    float specular_strength = 0.5f;
    vec3 view_dir = normalize(view_pos - vertex_pos_f);
    vec3 reflect_dir = reflect(-light_dir, vertex_normal_f);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * object_color;
    gl_FragColor = vec4(result, 1.0);
  }
);


static const char *simple_vert = _glsl(
  attribute vec3 vertex_pos;
  uniform mat4 mvp;
  void main() {
    gl_Position = mvp * vec4(vertex_pos, 1.0);
  }
);

static const char *simple_frag = _glsl(
  void main() {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
  }
);

static const char *colored_mesh_vert = _glsl(
  attribute vec3 vertex_pos;
  attribute vec3 vertex_color;
  varying vec3 vertex_color_f;
  uniform mat4 mvp;
  void main() {
    gl_Position = mvp * vec4(vertex_pos, 1.0);
    vertex_color_f = vertex_color;
  }
);

static const char *colored_mesh_frag = _glsl(
  varying vec3 vertex_color_f;
  void main() {
    gl_FragColor = vec4(vertex_color_f, 1.0);
  }
);

};

