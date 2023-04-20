#version 460 core

layout(location = 0) in vec4 in_color;
layout(location = 0) out vec4 o_color;

layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in vec3 v_eye;
layout(location = 4) in vec3 v_position;

uniform vec3 fogColor = vec3(0.04519f, 0.05781f, 0.09084f);
uniform float density = 0.0022f;

void main()
{



  //Placeholder directional light
  vec3 directional_light = vec3(-0.465f, 0.810f, 0.355f);
  directional_light = normalize(directional_light);

  //Ambient scale (I consider this like a 'mininium global white light')
  float ambient_scale = 0.1f;

  //Diffusion
  float diffuse_scale = max(dot(directional_light, in_normal), 0.0);


  //Specular
  float alpha = 500.0f;
  float ks = 1.0f;

  //Perfect Reflector Direction
  vec3 reflected_light = reflect(-directional_light, in_normal);
  vec3 dir_to_viewer = normalize(v_eye - v_position);

  float specular_scale = ks * max(pow(dot(reflected_light, dir_to_viewer), alpha), 0.0);   
  o_color = vec4(in_color.rgb * (ambient_scale + diffuse_scale + specular_scale), 1.0f);

  //Fog calculations

  float depth = gl_FragCoord.z / gl_FragCoord.w;
  float final_fog_factor = exp(-pow(density * depth, 2.0));
  final_fog_factor = clamp(final_fog_factor, 0.0, 1.0);
  o_color = vec4(mix(fogColor, o_color.rgb, final_fog_factor), 1.0f);
}