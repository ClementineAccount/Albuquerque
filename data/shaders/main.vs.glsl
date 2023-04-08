#version 460 core

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec4 iColor;

layout (location = 0) out vec4 oColor;

layout (location = 0) uniform mat4 uView;
layout (location = 1) uniform mat4 uProjection;


void main()
{
    gl_Position = uProjection * uView * vec4(iPosition, 1.0);
    oColor = iColor;
}