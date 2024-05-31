#ifndef PURRENGINE_SYSTEM_HPP_
#define PURRENGINE_SYSTEM_HPP_

#include "entityManager/Entity.hpp"
#include <vector>

namespace PurrfectEngine {

  class System {
  public:
    virtual ~System() = default;

    virtual void update(float deltaTime) = 0;

    void addEntity(Entity* entity) {
      mEntities.push_back(entity);
    }

  protected:
    std::vector<Entity*> mEntities;
  };

} // namespace PurrfectEngine

#endif // PURRENGINE_SYSTEM_HPP_
