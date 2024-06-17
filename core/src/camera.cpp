#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrCamera::purrCamera(purrTransform *transform):
    mTransform(transform)
  {  }

  purrCamera::~purrCamera() {
    
  }

  glm::mat4 purrCamera::getProjection() {
    glm::mat4 proj = glm::perspective(glm::radians(mSettings.fov), mSettings.aspectRatio, mSettings.nearPlane, mSettings.farPlane);
    proj[1][1] *= -1.0f;
    return proj;
  }

  glm::mat4 purrCamera::getView() {
    return glm::lookAt(mTransform->getPosition(), mTransform->getPosition() + mTransform->getForward(), mTransform->getUp());
  }

}