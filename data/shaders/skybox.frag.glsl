#version 460 core

layout(location = 0) in vec3 v_uv;
layout(location = 0) out vec4 o_color;
layout(binding = 0) uniform samplerCube cubeMap;

uniform vec3 fogColor = vec3(0.04519f, 0.05781f, 0.09084f);
uniform float fogScale = 15.0f; //The bigger this number the smaller the fog I don't fully understand why




void main()
{
    o_color = texture(cubeMap, v_uv);

    //Stole this idea from ThinMatrix
    float upper_limit_y_bound = 15.0f; //Figure out what unit this upper limit should be considered in to be honest
    float lower_limit_y_bound = 0.0f;

    float fog_factor = fogScale * ((v_uv.y - lower_limit_y_bound) / (upper_limit_y_bound - lower_limit_y_bound));
    fog_factor = clamp(fog_factor, 0.0f, 1.0f);

    o_color = mix(vec4(fogColor, 1.0f), o_color, fog_factor);
}