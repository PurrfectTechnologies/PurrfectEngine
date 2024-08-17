#ifndef   PURRENGINE_OBJECT_HPP_
#define   PURRENGINE_OBJECT_HPP_

#include <vector>
#include <algorithm>
#include <optional>
#include <unordered_map>

#include <stdint.h>

namespace PurrfectEngine {

  struct PUID {
  public:
    PUID();
    constexpr PUID(uint32_t id) : mId(id) {}

    friend bool operator==(PUID dis, PUID other) {
      return dis.mId == other.mId;
    }

    uint32_t operator()() {
      return mId;
    }
  private:
    uint32_t mId = 0;
  };

  class purrComponent {
  public:
    purrComponent();
    virtual ~purrComponent() = default;

    virtual const char *getName() = 0;
  private:
  };

  class purrScene;
  class purrCamera;
  class purrTransform;
  class purrAudioEngine;

  class purrCameraComp : public purrComponent {
  public:
    purrCameraComp(purrCamera *camera);
    virtual ~purrCameraComp() override;

    virtual const char *getName() override { return "cameraComponent"; }

    purrCamera *getCamera() const { return mCamera; }
  private:
    purrCamera *mCamera = nullptr;
  };

  class purrAudioComp : public purrComponent {
  public:
    purrAudioComp(purrAudioEngine *audioSource);
    virtual ~purrAudioComp() override;

    virtual const char *getName() override { return "audioComponent"; }

    purrAudioEngine *getAudio() const { return mAudio; };
  private:
    purrAudioEngine *mAudio = nullptr;
  };

  class purrObject {
  public:
    purrObject(purrScene *scene, purrTransform *transform);
    ~purrObject();

    bool addComponent(purrComponent* component);
    bool addComponent(purrCameraComp* component);
    purrComponent *getComponent(const char *name);
    bool removeComponent(const char *name);

    purrTransform *getTransform() const { return mTransform; }
    // void setTransform(purrTransform *trans) { mTransform = trans; }

    purrObject *newChild();

    bool isChild() const { return mParent.has_value(); }
    purrObject *getParent();
    bool isParent() const { return mChildren.size()>0; }
    std::vector<purrObject*> getChildren();

    PUID getUuid() const { return mUuid; }
  private:
    purrScene *mScene = nullptr;
    PUID mUuid{};
    purrTransform *mTransform = nullptr;

    std::optional<PUID> mParent{};
    std::vector<PUID> mChildren{};
    std::vector<const char *> mCompNames{};
    std::vector<purrComponent*> mComponents{};
  };

}

#endif // PURRENGINE_OBJECT_HPP_