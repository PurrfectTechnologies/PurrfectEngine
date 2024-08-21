#ifndef   PURRENGINE_SCENE_HPP_
#define   PURRENGINE_SCENE_HPP_

namespace PurrfectEngine {

  class purrObject;
  class purrScene {
    friend class purrObject;
  public:
    purrScene();
    purrScene(PUID cameraUuid);

    purrObject newObject();
    std::optional<purrObject> getObject(PUID uuid);
    std::vector<purrObject> getObjectsFlat() const;
    void removeObject(PUID uuid);

    std::optional<purrObject> getCamera();
  public:
    operator entt::registry&() { return mRegistry; }
  private:
    PUID mCameraUuid;
    std::unordered_map<PUID, entt::entity> mEntityMap{};
    entt::registry mRegistry;
  };

}

#endif // PURRENGINE_SCENE_HPP_