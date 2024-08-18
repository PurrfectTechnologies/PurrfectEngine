#include "PurrfectEngine/PurrfectEngine.hpp"
#include "PurrfectEngine/object.hpp"

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

  purrAudioSourceComp::purrAudioSourceComp(purrAudioSource *source):
    mAudioSource(source)
  {}

  purrAudioSourceComp::~purrAudioSourceComp() {
    
  }

  purrAudioListenerComp::purrAudioListenerComp():
    mAudioListener(new purrAudioListener(nullptr))
  {}

  purrAudioListenerComp::~purrAudioListenerComp() {
    
  }

  purrObject::purrObject(purrScene *scene, purrTransform *transform):
    mScene(scene), mTransform(transform)
  { assert(mTransform && "`transform` is a need, and I need to `transform`!"); }

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
    assert(mTransform && "`mTransform` is a need, and I need to `mTransform`!");
    component->getCamera()->setTransform(mTransform);
    return addComponent((purrComponent*)component);
  }

  bool purrObject::addComponent(purrAudioSourceComp* component) {
    assert(mTransform && "`mTransform` is a need, and I need to `mTransform`!");
    component->getSource()->setTransform(mTransform);
    return addComponent((purrComponent*)component);
  }

  bool purrObject::addComponent(purrAudioListenerComp* component) {
    assert(mTransform && "`mTransform` is a need, and I need to `mTransform`!");
    component->getListener()->setTransform(mTransform);
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

  purrObject *purrObject::getParent() {
    if (!mParent.has_value()) return nullptr;
    return mScene->getObject(mParent.value());
  }

  std::vector<purrObject*> purrObject::getChildren() {
    if (!isParent()) return {};
    std::vector<purrObject*> objects{};
    objects.reserve(mChildren.size());
    for (PUID puid: mChildren) objects.push_back(mScene->getObject(puid));
    return objects;
  }
}