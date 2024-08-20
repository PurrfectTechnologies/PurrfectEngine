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
  class purrAudioSource;
  class purrAudioListener;
  struct purrLight;
  class purrTransform;

  class purrCameraComp : public purrComponent {
  public:
    purrCameraComp(purrCamera *camera);
    virtual ~purrCameraComp() override;

    virtual const char *getName() override { return "cameraComponent"; }

    purrCamera *getCamera() const { return mCamera; }
  private:
    purrCamera *mCamera = nullptr;
  };

  class purrAudioSourceComp : public purrComponent {
  public:
    purrAudioSourceComp(purrAudioSource *source);
    virtual ~purrAudioSourceComp() override;

    virtual const char *getName() override { return "audioSourceComponent"; }

    purrAudioSource *getSource() const { return mAudioSource; };
  private:
    purrAudioSource *mAudioSource = nullptr;
  };

  class purrAudioListenerComp : public purrComponent {
  public:
    purrAudioListenerComp();
    virtual ~purrAudioListenerComp() override;

    virtual const char *getName() override { return "audioListenerComponent"; }

    purrAudioListener *getListener() const { return mAudioListener; };
  private:
    purrAudioListener *mAudioListener = nullptr;
  };

  class purrLightComp : public purrComponent {
    friend class purrObject;
  public:
    purrLightComp(glm::vec4 color);
    virtual ~purrLightComp() override;

    virtual const char *getName() override { return "lightComponent"; }

    const purrLight *getLight() const { return (const purrLight*)mLight; };
    void setColor(glm::vec4 color);
  private:
    void setPosition(glm::vec4 position);
  private:
    purrLight *mLight;
  };

  class purrObject {
    friend class purrScene;
  public:
    purrObject(purrScene *scene, purrTransform *transform);
    ~purrObject();

    bool addComponent(purrComponent* component);
    bool addComponent(purrCameraComp* component);
    bool addComponent(purrAudioSourceComp* component);
    bool addComponent(purrAudioListenerComp* component);
    bool addComponent(purrLightComp* component);
    purrComponent *getComponent(const char *name);
    bool removeComponent(const char *name);

    purrTransform *getTransform() const { return mTransform; }
    // void setTransform(purrTransform *trans) { mTransform = trans; }

    purrObject *getParent();
    bool isChild() const { return mParent.has_value(); }
    std::vector<purrObject*> getChildren();
    bool isParent() const { return mChildren.size()>0; }

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