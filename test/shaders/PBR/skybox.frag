#version 450

layout (location = 0) in vec3 inLocalPos;

layout (location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform samplerCube uSampler;

void main() {
	vec3 envColor = textureLod(uSampler, inLocalPos, 0.0).rgb;
  envColor = envColor / (envColor + vec3(1.0));
  envColor = pow(envColor, vec3(1.0/2.2)); 

  outColor = vec4(envColor, 1.0);
}