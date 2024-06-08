#version 450

layout (location = 0) in vec3 inPos;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 outLocalPos;
layout (location = 1) out vec2 outUV;

layout(binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
} cam;

void main() {
  outLocalPos = inPos;
  outUV = inUV;
  mat4 newView = mat4(mat3(cam.view));
  vec4 clipPos = cam.projection * newView * vec4(outLocalPos, 1.0);

  gl_Position = clipPos.xyzw;
}