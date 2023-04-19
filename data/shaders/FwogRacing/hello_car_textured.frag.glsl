#version 460 core

layout(binding = 0) uniform sampler2D s_baseColor;

layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_eye;
layout(location = 4) in vec3 v_position;

layout(location = 0) out vec4 o_color;

uniform vec3 fogColor = vec3(0.04519f, 0.05781f, 0.09084f);
uniform float density = 0.0015f;


void main()
{
  o_color = texture(s_baseColor, v_uv).rgba;

  float depth = gl_FragCoord.z / gl_FragCoord.w;
  float final_fog_factor = exp(-pow(density * depth, 2.0));
  final_fog_factor = clamp(final_fog_factor, 0.0, 1.0);
  o_color = vec4(mix(fogColor, o_color.rgb, final_fog_factor), 1.0f);
}