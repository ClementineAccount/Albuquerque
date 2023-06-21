#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_eye;
layout(location = 4) out vec3 v_position;

layout(binding = 0, std140) uniform UBO0
{
  mat4 viewProj;
  vec3 eyePos;
};

layout(binding = 1, std140) uniform UBO1
{
  mat4 model;
};

void main()
{
  v_position =  (model * vec4(a_pos, 1.0)).xyz;
  gl_Position =  viewProj * model * vec4(a_pos, 1.0);
  v_uv = a_uv;

  mat3 normalMatrix = inverse(transpose(mat3(model)));
  v_normal = normalize(normalMatrix * a_normal);

  v_eye = eyePos;
}