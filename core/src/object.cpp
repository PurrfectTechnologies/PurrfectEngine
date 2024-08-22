#include "PurrfectEngine/PurrfectEngine.hpp"
#include "PurrfectEngine/object.hpp"

#include <random>
#include <cstring>

namespace PurrfectEngine {

  purrObject::purrObject(purrScene *scene, entt::entity entity):
    mScene(scene), mHandle(entity)
  {}

  purrTransform *purrObject::fullTransform() {
    if (!hasComponent<purrTransform>()) return nullptr;
    purrTransform trans = getComponent<purrTransform>();

    glm::mat4 transMat = trans.getTransform();
    purrObject obj = *this;
    while (obj.hasComponent<ParentComponent>()) {
      ParentComponent parent = obj.getComponent<ParentComponent>();
      obj = purrObject{mScene, parent.parent};
      if (!obj.hasComponent<purrTransform>()) continue;
      transMat = obj.getComponent<purrTransform>().getTransform() * transMat;
    }

    return new purrTransform(transMat);
  }

  purrObject purrObject::createChild() {
    purrObject child = mScene->newObject();
    child.addComponent<ParentComponent>(mHandle);
    if (hasComponent<ChildrenComponent>()) {
      ChildrenComponent &children = getComponent<ChildrenComponent>();
      children.children.push_back(child.mHandle);
    } else {
      addComponent<ChildrenComponent>(std::vector<entt::entity>{child.mHandle});
    }

    return child;
  }

}