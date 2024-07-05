layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;

layout (binding = 0) uniform CameraBuffer {
  mat4 projection;
  mat4 view;
} camera;

struct ObjectData {
	mat4 model;
};

layout(std140, binding = 1) readonly buffer ObjectBuffer{
	ObjectData objects[];
} objectBuffer;

layout (push_constant) uniform constants {
  ivec4 data; // w = model #
} pc;