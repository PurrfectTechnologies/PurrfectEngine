#include "PurrfectEngine/PurrfectEngine.hpp"

#include <algorithm>

namespace PurrfectEngine {

  purrScene::purrScene()
  {}

  purrScene::purrScene(PUID uuid):
    mUuid(uuid)
  {}

  purrObject purrScene::newObject() {
    entt::entity entity = mRegistry.create();
    mEntityMap.insert(obj.getUuid(), entity);
    return purrObject{this, entity};
  }

  std::optional<purrObject> purrScene::getObject(PUID uuid) {
    if (mEntityMap.find(uuid) == mEntityMap.end())
      return {};
    return {purrObject{this, mEntityMap[uuid]}};
  }
  
  void purrScene::removeObject(PUID uuid) {
    if (auto it = mEntityMap.find(uuid); it != mEntityMap.end()) {
      mRegistry.remove(it->second);
      mEntityMap.erase(it);
    }
  }

  std::optional<purrObject> purrScene::getCamera() {
    return {getObject(mCameraUuid)};
  }

}