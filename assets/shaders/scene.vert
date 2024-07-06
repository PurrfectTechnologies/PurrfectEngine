#version 450

#include "./vertex3D.glsl"

void main() {
  mat4 modelMat = objectBuffer.objects[pc.data.w].model;
  mat4 transMat = camera.projection * camera.view * modelMat;
  gl_Position = transMat * vec4(inPos, 1.0);
  outColor    = inColor;
  outUV       = inUV;
  outNormal   = inNormal;
}