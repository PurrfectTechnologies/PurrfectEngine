#ifndef   PURRENGINE_TRANSFORM_HPP_
#define   PURRENGINE_TRANSFORM_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace PurrfectEngine {

  class purrTransform {
  public:
    purrTransform(glm::vec3 position = glm::vec3(0.0f), glm::quat rotation = glm::quat(), glm::vec3 scale = glm::vec3(1.0f));
    purrTransform(glm::mat4 transform);
    ~purrTransform();

    void update();

    void setTransform(glm::mat4 trans);
  public:
    glm::vec3 getForward();
    glm::vec3 getRight();
    glm::vec3 getUp();
  public:
    void setPosition(glm::vec3 pos) { mPos = pos; }
    void setRotation(glm::quat rot) { mRot = rot; }
    void setScale(glm::vec3 scale)  { mScale = scale; }

    glm::vec3   getPosition() const { return mPos; }
    glm::quat   getRotation() const { return mRot; }
    glm::vec3   getScale()    const { return mScale; }

    glm::mat4 getTransform() { update(); return mTransform; }
  private:
    glm::vec3 mPos = glm::vec3(), mScale = glm::vec3();
    glm::quat mRot = glm::quat();

    glm::mat4 mTransform = glm::mat4();
  };

}

#endif // PURRENGINE_TRANSFORM_HPP_