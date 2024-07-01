#version 450

// layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

void main() {
  // outColor = vec4(texture(uSampler, inUV).rgb, 1.0f);
  outColor = vec4(inColor, 1.0);
}