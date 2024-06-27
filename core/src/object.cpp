#include "PurrfectEngine/PurrfectEngine.hpp"

#include <random>
#include <cstring>

namespace PurrfectEngine {

  static std::random_device sRandomDevice;
	static std::mt19937_64 sEngine(sRandomDevice());
	static std::uniform_int_distribution<uint32_t> sUniformDistribution;

  PUID::PUID():
    mId(sUniformDistribution(sEngine))
  {}

  purrComponent::purrComponent()
  {}

  // purrMeshComp::purrMeshComp(purrMesh *mesh):
  //   mMesh(mesh)
  // { assert(mesh); }

  // // purrMeshComp::purrMeshComp(purrMesh2D *mesh):
  // //   mMesh2D(mesh)
  // // {}

  // purrMeshComp::purrMeshComp(bool is2D, const char *filename)
  // {
  //   assert(!is2D && "2D not supported yet!");
  //   // if (is2D) {
  //   //   mMesh2D = new purrMesh2D();
  //   //   mMesh2D->initialize(filename);
  //   // } else {
  //     mMesh = new purrMesh();
  //     mMesh->initialize(filename);
  //   // }
  // }

  // purrMeshComp::~purrMeshComp() {
  //   if (mMesh) delete mMesh;
  //   // if (mMesh2D) delete mMesh2D;
  // }

  // purrLightComp::purrLightComp(purrLight *light):
  //   mLight(light)
  // { assert(light); }

  // purrLightComp::~purrLightComp() {
  //   delete mLight;
  // }

  purrCameraComp::purrCameraComp(purrCamera *camera):
    mCamera(camera)
  { assert(camera); }

  purrCameraComp::~purrCameraComp() {
    delete mCamera;
  }

  purrAudioComp::purrAudioComp(purrAudioEngine *audioSource):
    mAudio(audioSource)
  { assert(audioSource); }

  purrAudioComp::~purrAudioComp() {
    if (mAudio) {
      delete mAudio;
    }
  }

  purrObject::purrObject(purrTransform *transform):
    mTransform(transform)
  {}

  purrObject::~purrObject() {
    delete mTransform;
    for (purrComponent *comp: mComponents) delete comp;
  }

  bool purrObject::addComponent(purrComponent* component) {
    if (std::find(mCompNames.begin(), mCompNames.end(), component->getName()) != mCompNames.end())
      return false;
    mCompNames.push_back(component->getName());
    mComponents.push_back(component);
    return true;
  }

  bool purrObject::addComponent(purrCameraComp* component) {
    component->getCamera()->setTransform(mTransform);
    return addComponent((purrComponent*)component);
  }

  purrComponent *purrObject::getComponent(const char *name) {
    auto it = std::find_if(mCompNames.begin(), mCompNames.end(), [&](const char* str) {
      return strcmp(str, name) == 0;
    });
    if (it == mCompNames.end()) return nullptr;
    return mComponents.at(it-mCompNames.begin());
  }

  bool purrObject::removeComponent(const char *name) {
    std::vector<const char*>::iterator it;
    if ((it = std::find(mCompNames.begin(), mCompNames.end(), name)) == mCompNames.end()) return false;
    mCompNames.erase(it);
    mComponents.erase(mComponents.begin()+(it-mCompNames.begin()));
    return true;
  }

}