#version 450

#include "./light.glsl"

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform CameraBuffer {
  mat4 projection;
  mat4 view;
  vec4 pos;
} camera;

void main() {
	// vec4 diffuseTex = texture(sampler2D(uTextures[0], uSampler), inUV);
	// vec4 specularTex = texture(sampler2D(uTextures[1], uSampler), inUV);

  float ambientLight = 0.2;
  vec3 diffuseLight = vec3(0.0);
  vec3 specularLight = vec3(0.0);

	int shininess = 128;

  vec3 norm = normalize(inNormal);
  for (int i = 0; i < lightBuffer.lightCount.x; ++i) {
    Light light = lightBuffer.lights[i];

    vec3 lightDir = normalize(vec3(light.pos) - inPosition);
	  diffuseLight += max(dot(lightDir, norm), 0.0) * vec3(light.color);// * diffuseTex.rgb;

    vec3 viewDir    = normalize(vec3(camera.pos) - inPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    specularLight  += pow(max(dot(norm, halfwayDir), 0.0), shininess) * vec3(light.color);// * specularTex.rgb;
  }

	vec3 color = vec3(1.0);//diffuseTex.rgb;
	outColor = vec4((vec3(ambientLight + diffuseLight + specularLight) * color), 1.0);
	// outColor = vec4(inUV, 0.0, 1.0);
}