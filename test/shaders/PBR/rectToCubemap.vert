#version 450

layout (location = 0) in vec3 inPos;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 outLocalPos;

layout(binding = 0) uniform Camera {
  mat4 projection;
  mat4 view;
} cam;

void main() {
  outLocalPos = inPos;

	// mat4 newView = cam.view;
	// newView[3][0] = 0.0;
	// newView[3][1] = 0.0;
	// newView[3][2] = 0.0;

  gl_Position = cam.projection * cam.view * vec4(outLocalPos, 1.0);
}