#version 450

layout (location = 0) in vec3 inLocalPos;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D uSampler;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main() {
	vec2 uv = SampleSphericalMap(normalize(inLocalPos));
  vec3 color = texture(uSampler, -uv).rgb;
  outColor = vec4(color, 1.0);
}