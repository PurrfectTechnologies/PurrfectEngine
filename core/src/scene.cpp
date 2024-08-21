#include "PurrfectEngine/PurrfectEngine.hpp"

#include <algorithm>

namespace PurrfectEngine {

  purrScene::purrScene()
  {}

  purrScene::purrScene(PUID cameraUuid):
    mCameraUuid(cameraUuid)
  {}

  purrObject purrScene::newObject() {
    entt::entity entity = mRegistry.create();
    purrObject obj{this, entity}; 
    mEntityMap.insert({obj.getUuid(), entity});
    return obj;
  }

  std::optional<purrObject> purrScene::getObject(PUID uuid) {
    if (mEntityMap.find(uuid) == mEntityMap.end())
      return {};
    return {purrObject{this, mEntityMap[uuid]}};
  }

  std::vector<purrObject> purrScene::getObjectsFlat() const {
    std::vector<purrObject> objects;
    for (const auto& [uuid, entity] : mEntityMap) {
      objects.push_back(purrObject{const_cast<purrScene*>(this), entity});
    }
    return objects;
  }



  
  void purrScene::removeObject(PUID uuid) {
    if (auto it = mEntityMap.find(uuid); it != mEntityMap.end()) {
      mRegistry.destroy(it->second);
      mEntityMap.erase(it);
    }
  }

  std::optional<purrObject> purrScene::getCamera() {
    return {getObject(mCameraUuid)};
  }

}