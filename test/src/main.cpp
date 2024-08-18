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
      purrObject *obj = nullptr;
      if (!purrMesh3D::loadModel("./assets/models/pyramid.obj", mScene, &obj)) return 1;
      obj->getTransform()->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
      mScene->addObject(obj);
    }

    purrObject *camObj = nullptr;
    { // Initialize camera
      camObj = mScene->newObject();
      if (!camObj) return false;
      camObj->addComponent(new purrCameraComp(new purrCamera()));
      mScene->addObject(camObj);
      mScene->setCamera(camObj);
    }

    mAudioListener = new purrAudioListener(new purrTransform());
    mAudioSource = purrAudioEngine::getInstance()->newSource(new purrTransform());
    // We don't have an example sound effect :p
    // if (!purrAudioEngine::getInstance()->load("./assets/sound/sound.wav", &mAudioSource) ||
    //     !mAudioSource->initialize()) return false;
    mAudioSource->play();

    purrRenderer3D *renderer = (purrRenderer3D*)purrRenderer::getInstance();
    renderer->setScene(mScene);
    return renderer->update();
  }

  virtual bool update(float dt) override {
    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int y = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    if (mScene->getCamera() && mScene->getCamera()->getTransform()) {
      glm::vec3 rot = glm::eulerAngles(mScene->getCamera()->getTransform()->getRotation());
      rot.x -= y * dt;
      rot.y -= x * dt;
      mScene->getCamera()->getTransform()->setRotation(glm::quat(rot));
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