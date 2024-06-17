#ifndef   PURRENGINE_OBJECT_HPP_
#define   PURRENGINE_OBJECT_HPP_

#include <vector>
#include <algorithm>
#include <unordered_map>

#include <stdint.h>

namespace PurrfectEngine {

  struct PUID {
    PUID();

    PUID(uint32_t id):
      mId(id)
    {}

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

  // class purrMesh;
  // class purrMeshComp : public purrComponent {
  // public:
  //   purrMeshComp(purrMesh *mesh);
  //   // purrMeshComp(purrMesh2D *mesh);
  //   purrMeshComp(bool is2D, const char *filename);
  //   virtual ~purrMeshComp() override;

  //   virtual const char *getName() override { return "meshComponent"; }

  //   purrMesh *getMesh() const { return mMesh; }
  // private:
  //   bool is2D = false;
  //   purrMesh *mMesh = nullptr;
  //   // purrMesh2D *mMesh2D = nullptr;
  // };

  // class purrLight;
  // class purrLightComp : public purrComponent {
  // public:
  //   purrLightComp(purrLight *light);
  //   virtual ~purrLightComp() override;

  //   virtual const char *getName() override { return "lightComponent"; }

  //   purrLight *getLight() const { return mLight; }
  // private:
  //   purrLight *mLight = nullptr;
  // };

  class purrCameraComp : public purrComponent {
  public:
    purrCameraComp(purrCamera *camera);
    virtual ~purrCameraComp() override;

    virtual const char *getName() override { return "cameraComponent"; }

    purrCamera *getCamera() const { return mCamera; }
  private:
    purrCamera *mCamera = nullptr;
  };

  class purrObject {
  public:
    purrObject(purrTransform *transform = new purrTransform());
    ~purrObject();

    bool addComponent(purrComponent* component);
    bool addComponent(purrCameraComp* component);
    purrComponent *getComponent(const char *name);
    bool removeComponent(const char *name);

    purrTransform *getTransform() const { return mTransform; }
    // void setTransform(purrTransform *trans) { mTransform = trans; }

    PUID getUuid() const { return mUuid; }
  private:
    PUID mUuid{};
    purrTransform *mTransform = new purrTransform();

    std::vector<const char *> mCompNames{};
    std::vector<purrComponent*> mComponents{};
  };

}

#endif // PURRENGINE_OBJECT_HPP_