#version 460 core

layout (location = 0) out vec4 oPixel;

layout (location = 0) in vec4 iColor;

void main()
{
    oPixel = iColor;
}