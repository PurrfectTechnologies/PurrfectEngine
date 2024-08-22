#include "PurrfectEngine/PurrfectEngine.hpp"

#include <algorithm>

namespace PurrfectEngine {

  purrScene::purrScene()
  {}

  purrScene::purrScene(PUID cameraUuid):
    mCameraUuid(cameraUuid)
  {}

  purrScene::~purrScene() {
    mRegistry.clear();
  }

  purrObject purrScene::newObject() {
    entt::entity entity = mRegistry.create();
    purrObject obj{this, entity}; 
    obj.addComponent<purrTransform>();
    mEntityMap.insert({obj.getUuid(), entity});
    return obj;
  }

  std::optional<purrObject> purrScene::getObject(PUID uuid) {
    if (mEntityMap.find(uuid) == mEntityMap.end())
      return {};
    return {purrObject{this, mEntityMap[uuid]}};
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