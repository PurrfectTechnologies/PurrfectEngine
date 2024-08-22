#include "PurrfectEngine/PurrfectEngine.hpp"
#include "PurrfectEngine/object.hpp"

#include <random>
#include <cstring>

namespace PurrfectEngine {

  purrObject::purrObject(purrScene *scene, entt::entity entity):
    mScene(scene), mHandle(entity)
  {}

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