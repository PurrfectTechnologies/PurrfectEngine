#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrCamera::purrCamera(purrTransform *transform):
    mTransform(transform)
  {  }

  purrCamera::~purrCamera() {
    
  }

  glm::mat4 purrCamera::getProjection() {
    return glm::perspective(glm::radians(mSettings.fov), mSettings.aspectRatio, mSettings.nearPlane, mSettings.farPlane);
  }

  glm::mat4 purrCamera::getView() {
    return glm::lookAt(mTransform->getPosition(), mTransform->getPosition() + mTransform->getForward(), glm::vec3(0.0f, 1.0f, 0.0f)); // TODO: Maybe let user to choose up direction?
  }

}