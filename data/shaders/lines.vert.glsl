#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

layout(location = 0) out vec3 v_color;

layout(binding = 0, std140) uniform UBO0
{
  mat4 viewProj;
};

void main()
{
  gl_Position =  viewProj * vec4(a_pos, 1.0);
  v_color = a_color;
}