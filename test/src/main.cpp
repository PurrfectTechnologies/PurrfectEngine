#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp: public purrApp {
public:
  testApp():
    purrApp(purrAppCreateInfo{"PurrEngineTest"}, {
      new purrAppRendererExt(new purrRenderer3D(), purrWindowInitInfo{"PurrfectEngine - Test", 1920, 1080}, purrRendererInitInfo{{ VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" }}),
      new purrAppAudioExt()
    })
  {}

  ~testApp() {

  }
protected:
  virtual bool initialize() override {
    input::GetEventHandler()->on("keyEvent", [](purrEvent *event){
      purrKeyEvent *keyEvent = (purrKeyEvent*)event;
    });

    input::GetEventHandler()->on("mouseButtonEvent", [](purrEvent *event){
      purrMouseBtnEvent *mouseEvent = (purrMouseBtnEvent*)event;
    });

    input::GetEventHandler()->on("mouseMoveEvent", [](purrEvent *event){
      purrMouseMoveEvent *mouseEvent = (purrMouseMoveEvent*)event;
    });

    mScene = new purrScene();
    { // Initialize object
      purrObject *root = mScene->newObject();
      root->getTransform()->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
      purrObject *meshObj = mScene->newChildObject(root);
      if (!purrMesh3D::loadModel("./assets/models/pyramid.obj", mScene, &meshObj)) return 1;
      mScene->addObject(meshObj);
      purrObject *lightObj = mScene->newChildObject(root);
      lightObj->addComponent(new purrLightComp(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
      lightObj->getTransform()->setPosition(glm::vec3(0.0f, 0.0f, -1.0f));
    }

    { // Initialize camera
      purrObject *obj = mScene->newObject();
      if (!obj) return false;
      obj->addComponent(new purrCameraComp(new purrCamera()));
      mScene->addObject(obj);
      mScene->setCamera(obj);
    }

    { // Initialize audio listener
      purrObject *obj = mScene->newObject();
      if (!obj) return false;
      obj->addComponent(new purrAudioListenerComp());
      mScene->addObject(obj);
      mScene->setAudioListener(obj);
    }

    { // Initialize audio source
      purrObject *obj = mScene->newObject();
      if (!obj) return false;

      purrAudioSource *source = purrAudioEngine::getInstance()->newSource();
      // We don't have an example sound effect :p
      // if (!purrAudioEngine::load("./assets/sound/sound.wav", &source) ||
      //     !source->initialize()) return false;
      source->play();

      obj->addComponent(new purrAudioSourceComp(source));
      mScene->addObject(obj);
    }

    purrRenderer3D *renderer = (purrRenderer3D*)purrRenderer::getInstance();
    renderer->setScene(mScene);
    return renderer->update();
  }

  virtual bool update(float dt) override {
    int x = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);
    int z = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);

    if (mScene->getCamera() && mScene->getCamera()->getTransform()) {
      mScene->getCamera()->getTransform()->setPosition(mScene->getCamera()->getTransform()->getPosition() + (glm::vec3((float)x, 0.0f, (float)z * dt)));
    }

    return true;
  }

  virtual void cleanup() override {
    delete mScene;
    delete mAudioListener;
  }
private:
  purrScene *mScene = nullptr;
  purrAudioListener *mAudioListener = nullptr;
  purrAudioSource *mAudioSource = nullptr;
};

purrApp *PurrfectEngine::CreateApp() {
  return new testApp();
}