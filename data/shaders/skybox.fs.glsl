#version 460 core

layout(location = 0) in vec3 v_uv;
layout(location = 0) out vec4 o_color;
layout(binding = 0) uniform samplerCube cubeMap;

void main()
{
    o_color = texture(cubeMap, v_uv);
}