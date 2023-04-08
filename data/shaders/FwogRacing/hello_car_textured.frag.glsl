#version 460 core

layout(location = 2) in vec2 v_uv;

layout(binding = 0) uniform sampler2D s_baseColor;
layout(location = 0) out vec4 o_color;

void main()
{
  o_color = texture(s_baseColor, v_uv).rgba;
}