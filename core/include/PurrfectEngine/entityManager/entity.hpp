#ifndef PURRENGINE_ENTITYMANAGER_ENTITY_HPP_
#define PURRENGINE_ENTITYMANAGER_ENTITY_HPP_

#include "PurrfectEngine/object.hpp"
#include "PurrfectEngine/transform.hpp"

namespace PurrfectEngine {

  class Entity : public purrObject {
  public:
    Entity(purrTransform* transform = new purrTransform()) : purrObject(transform) {}

  };

} // namespace PurrfectEngine

#endif // PURRENGINE_ENTITYMANAGER_ENTITY_HPP_
