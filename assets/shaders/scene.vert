#version 450

#include "./vertex3D.glsl"

void main() {
  mat4 modelMat = objectBuffer.objects[pc.data.w].model;
  outPosition   = vec3(modelMat * vec4(inPos, 1.0));
  gl_Position   = camera.projection * camera.view * vec4(outPosition, 1.0);
  outColor      = inColor;
  outUV         = inUV;
  outNormal     = mat3(inverse(transpose(modelMat))) * inNormal;
}