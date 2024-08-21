#ifndef   PURRENGINE_SCENE_HPP_
#define   PURRENGINE_SCENE_HPP_

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
    std::vector<purrObject*> getObjectsFlat() const { return mObjects; }
    // Get only root objects.
    std::vector<purrObject*> getObjects() const;
  private:
    PUID mUuid{};
    std::vector<PUID> mUuids{};
    std::vector<purrObject*> mObjects{};
    purrObject *mCameraObject = nullptr;
    purrObject *mAudioListenerObject = nullptr;
  };

}

#endif // PURRENGINE_SCENE_HPP_