struct Light {
  vec4 pos;
  vec4 color;
};

layout(std140, binding = 2) readonly buffer LightsBuffer {
	ivec4 lightCount;
  Light lights[];
} lightBuffer;