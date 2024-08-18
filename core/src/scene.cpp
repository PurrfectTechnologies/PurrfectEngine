#include "PurrfectEngine/PurrfectEngine.hpp"

#include <algorithm>

namespace PurrfectEngine {

  purrScene::purrScene()
  {}

  purrScene::purrScene(PUID uuid, std::vector<purrObject*> objects, purrObject *cameraObject, purrObject *audioListenerObject):
    mUuid(uuid), mCameraObject(cameraObject), mAudioListenerObject(audioListenerObject)
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

  bool purrScene::setAudioListener(purrObject *object) {
    if (!purrAudioEngine::getInstance()) return false;
    purrAudioEngine::getInstance()->setListener(((purrAudioListenerComp*)((mAudioListenerObject = object)->getComponent("audioListenerComponent")))->getListener());
    return true;
  }

  purrObject *purrScene::newObject() {
    purrObject *obj = new purrObject(this, new purrTransform());
    if (!addObject(obj)) {
      delete obj; return nullptr;
    }
    return obj;
  }

  purrObject *purrScene::newChildObject(purrObject *parent) {
    if (!parent) return nullptr;
    purrObject *obj = newObject();
    if (!obj) return nullptr;
    parent->mChildren.push_back(obj->mUuid);
    obj->mParent = parent->mUuid;
    return obj;
  }

  std::vector<purrObject*> purrScene::getObjects() const {
    std::vector<purrObject*> copy = mObjects;
    copy.erase(std::remove_if(copy.begin(), copy.end(), [](purrObject *obj){ return obj->isChild(); }), copy.end());
    return copy;
  }

}