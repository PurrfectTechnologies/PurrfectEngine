#version 450

layout(binding = 0) uniform CameraUBO {
  mat4 projection;
  mat4 view;
} camera;

layout(std140,set = 1, binding = 0) readonly buffer ModelBuffer {
	mat4 models[];
} models;

layout(push_constant) uniform constants {
	uint objectIndex;
} pc;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;

void main() {
  gl_Position = camera.projection * camera.view * models.models[pc.objectIndex] * vec4(inPos, 1.0);
  outColor = inColor;
  outUV = inUV;
}