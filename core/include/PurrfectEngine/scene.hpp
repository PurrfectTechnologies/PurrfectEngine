#ifndef   PURRENGINE_SCENE_HPP_
#define   PURRENGINE_SCENE_HPP_

#include <vector>
#include "PurrfectEngine/object.hpp"

namespace PurrfectEngine {
  class purrScene {
    friend class purrObject;
  public:
    purrScene();
    purrScene(PUID uuid, std::vector<purrObject*> objects, purrObject *cameraObject, purrObject *audioListenerObject);
    ~purrScene();

    bool addObject(purrObject *obj);
    bool addObjects(std::vector<purrObject*> objects);
    purrObject *getObject(PUID uuid);
    bool removeObject(PUID uuid);

    void setCamera(purrObject *object) { mCameraObject = object; }
    purrObject *getCamera() const { return mCameraObject; }

    bool setAudioListener(purrObject *object);
    purrObject *getAudioListener() const { return mAudioListenerObject; }

    purrObject *newObject();
    purrObject *newChildObject(purrObject *parent);
  public:
    std::vector<purrObject*> getObjects() const { return mObjects; }
  private:
    PUID mUuid{};
    std::vector<PUID> mUuids{};
    std::vector<purrObject*> mObjects{};
    std::vector<PUID> mChildrenUuids{};
    std::vector<purrObject*> mChildrenObjects{};
    purrObject *mCameraObject = nullptr;
    purrObject *mAudioListenerObject = nullptr;
  };

}

#endif // PURRENGINE_SCENE_HPP_