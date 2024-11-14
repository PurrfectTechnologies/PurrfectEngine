#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrCamera::purrCamera()
  {}

  purrCamera::~purrCamera() {
    
  }

  glm::mat4 purrCamera::getProjection() {
    glm::mat4 proj = glm::perspective(glm::radians(mSettings.fov), mSettings.aspectRatio, mSettings.nearPlane, mSettings.farPlane);
    proj[1][1] *= -1.0f;
    return proj;
  }

  glm::mat4 purrCamera::getView(purrTransform transform) {
    return glm::lookAt(transform.getPosition(), transform.getPosition() + transform.getForward(), transform.getUp());
  }

}