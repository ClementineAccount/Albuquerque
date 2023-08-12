#version 460 core

layout(location = 0) in vec3 pos;

void main()
{
  gl_Position = vec4(pos.x * 0.5f, pos.y * 0.5f, pos.z, 1.0);
}