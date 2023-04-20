#version 460 core
layout(location = 0) in vec3 a_position;

//In skyboxes, the position is also the uv coordinates because GLSL is quirky
layout(location = 0) out vec3 v_uv;

layout(binding = 0, std140) uniform UBO0
{
  mat4 viewProj;
  vec3 eyePos;
};

void main()
{
    gl_Position = viewProj * vec4(a_position, 1.0); 
	v_uv = a_position;
}