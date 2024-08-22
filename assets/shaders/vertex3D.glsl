layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outPosition;

layout (binding = 0) uniform CameraBuffer {
  mat4 projection;
  mat4 view;
  vec4 pos;
} camera;

layout (push_constant) uniform constants {
  mat4 transform;
} pc;