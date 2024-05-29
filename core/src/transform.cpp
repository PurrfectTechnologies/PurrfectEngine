#include "PurrfectEngine/PurrfectEngine.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace PurrfectEngine {

  purrTransform::purrTransform(glm::vec3 position, glm::quat rotation, glm::vec3 scale):
    mPos(position), mRot(rotation), mScale(scale)
  { update(); }

  purrTransform::purrTransform(glm::mat4 transform)
  { setTransform(transform); }

  purrTransform::~purrTransform() {

  }

  void purrTransform::update() {
    mTransform = glm::mat4(glm::mat3(mRot)) * glm::scale(glm::translate(glm::mat4(1.0f), mPos), mScale);
  }

  void purrTransform::setTransform(glm::mat4 trans) {
    mPos = mTransform[3];
    mScale.x = glm::length(glm::vec3(mTransform[0]));
    mScale.y = glm::length(glm::vec3(mTransform[1]));
    mScale.z = glm::length(glm::vec3(mTransform[2]));
    mRot = glm::quat_cast(mTransform);
  }
  
  glm::vec3 purrTransform::getForward() {
    return glm::rotate(mRot, glm::vec3(0.0f, 0.0f, 1.0f));
  }

  glm::vec3 purrTransform::getRight() {
    return glm::rotate(mRot, glm::vec3(-1.0f, 0.0f, 0.0f));
  }

  glm::vec3 purrTransform::getUp() {
    return glm::rotate(mRot, glm::vec3(0.0f, 1.0f, 0.0f));
  }

}