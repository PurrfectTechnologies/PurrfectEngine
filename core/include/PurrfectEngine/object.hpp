#ifndef   PURRENGINE_OBJECT_HPP_
#define   PURRENGINE_OBJECT_HPP_

#include <stdint.h>

namespace PurrfectEngine {

  // Component for child objects.
  struct ParentComponent {
    entt::entity parent = entt::null;
  };

  // Component for objects with children.
  struct ChildrenComponent {
    std::vector<entt::entity> children;
  };

  class purrScene;

  class purrObject {
    friend class purrScene;
  public:
    purrObject(purrScene *scene, entt::entity entity);

    template <typename T, typename... Args>
    T &addComponent(Args &&...args) {
      return mScene->mRegistry.emplace<T>(mHandle, std::forward<Args>(args)...);
    }

    template <typename T>
    T &getComponent() {
      return mScene->mRegistry.get<T>(mHandle);
    }

    template <typename T>
    bool hasComponent() {
      return mScene->mRegistry.any_of<T>(mHandle);
    }

    template <typename T>
    void removeComponent() {
      mScene->mRegistry.remove<T>(mHandle);
    }

    purrObject createChild();

    PUID getUuid() const { return mUuid; }
  private:
    
  private:
    purrScene *mScene;
    entt::entity mHandle;
    PUID mUuid{};
  };

}

#endif // PURRENGINE_OBJECT_HPP_