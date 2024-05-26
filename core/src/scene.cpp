#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrScene::purrScene()
  {}

  purrScene::purrScene(PUID uuid, std::vector<purrObject*> objects, purrObject *cameraObject):
    mUuid(uuid), mCameraObject(cameraObject)
  { addObjects(objects); }

  purrScene::~purrScene() {
    for (purrObject *obj: mObjects) delete obj;
  }

  bool purrScene::addObject(purrObject *obj) {
    if (std::find(mUuids.begin(), mUuids.end(), obj->getUuid()) != mUuids.end())
      return false;
    mUuids.push_back(obj->getUuid());
    mObjects.push_back(obj);
    return true;
  }

  bool purrScene::addObjects(std::vector<purrObject*> objects) {
    bool result = true;
    for (purrObject *obj: objects) {
      if (!addObject(obj)) result = false;
    }
    return result;
  }

  purrObject *purrScene::getObject(PUID uuid) {
    std::vector<PUID>::iterator it;
    if ((it = std::find(mUuids.begin(), mUuids.end(), uuid)) == mUuids.end()) return nullptr;
    return mObjects.at(it-mUuids.begin());
  }

  bool purrScene::removeObject(PUID uuid) {
    std::vector<PUID>::iterator it;
    if ((it = std::find(mUuids.begin(), mUuids.end(), uuid)) == mUuids.end()) return false;
    mUuids.erase(it);
    mObjects.erase(mObjects.begin()+(it-mUuids.begin()));
    return true;
  }

  purrObject *purrScene::newObject() {
    purrObject *obj = new purrObject();
    if (!addObject(obj)) {
      delete obj; return nullptr;
    }
    return obj;
  }

}