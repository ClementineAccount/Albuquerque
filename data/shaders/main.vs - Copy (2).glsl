#version 460 core

layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iUv;
layout (location = 3) in vec4 iTangent;

layout (location = 0) out vec2 oUvs;
layout (location = 1) out flat uint oBaseColorIndex;

layout (location = 0) uniform mat4 uView;
layout (location = 1) uniform mat4 uProjection;


void main()
{
    gl_Position = uProjection * uView * vec4(iPosition, 1.0);
}