#version 450

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(texture(uSampler, inUV).rgb, 1.0f);
}