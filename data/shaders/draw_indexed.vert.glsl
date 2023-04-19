//Adapted from Fwog's example: https://github.com/JuanDiegoMontoya/Fwog/blob/main/example/shaders/SceneDeferred.vert.glsl
//No reason to do anything different yet

#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

layout(location = 0) out vec4 o_color;
layout(location = 1) out vec3 v_normal;
layout(location = 2) out vec2 v_uv;
layout(location = 3) out vec3 v_eye;
layout(location = 4) out vec3 v_position;



layout(binding = 0, std140) uniform UBO0
{
  mat4 viewProj;
  vec3 eyePos;
};

struct ObjectUniforms
{
  mat4 model;
  vec4 color;
};

layout(binding = 1, std430) readonly buffer SSBO0
{
  ObjectUniforms objects[];
};

void main()
{
  int i = gl_InstanceID;
  v_position = (objects[i].model * vec4(a_pos, 1.0)).xyz;
  v_normal = normalize(inverse(transpose(mat3(objects[i].model))) * a_normal);
  v_uv = a_uv;
  o_color = objects[i].color.rgba;
  gl_Position = viewProj * vec4(v_position, 1.0);
  v_eye = eyePos;
}