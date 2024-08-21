#include "PurrfectEngine/PurrfectEngine.hpp"
#include "PurrfectEngine/object.hpp"

#include <random>
#include <cstring>

namespace PurrfectEngine {

  purrObject::purrObject(purrScene *scene, entt::entity entity):
    mScene(scene), mHandle(entity)
  {}

  purrObject purrObject::createChild() {
    entt::entity entity = mScene->mRegistry.create();
    purrObject child{mScene, entity};
    child.addComponent<ParentComponent>(mHandle);
    if (hasComponent<ChildrenComponent>()) {
      ChildrenComponent &children = getComponent<ChildrenComponent>();
      children.children.push_back(entity);
    } else {
      addComponent<ChildrenComponent>(std::vector<entt::entity>{entity});
    }

    return child;
  }

}