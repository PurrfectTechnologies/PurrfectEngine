#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp : public purrApp {
public:
  testApp() :
    purrApp(purrAppCreateInfo{ "PurrEngineTest" }, {
      new purrAppRendererExt(new purrRenderer3D(), purrWindowInitInfo{ "PurrfectEngine - Test", 1920, 1080 }, purrRendererInitInfo{ { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" } }),
      new purrAppAudioExt()
    })
  {}

  ~testApp() {
    cleanup();
  }

protected:
  virtual bool initialize() override {
    input::GetEventHandler()->on("keyEvent", [](purrEvent* event) {
      purrKeyEvent* keyEvent = (purrKeyEvent*)event;
    });

    input::GetEventHandler()->on("mouseButtonEvent", [](purrEvent* event) {
      purrMouseBtnEvent* mouseEvent = (purrMouseBtnEvent*)event;
    });

    input::GetEventHandler()->on("mouseMoveEvent", [](purrEvent* event) {
      purrMouseMoveEvent* mouseEvent = (purrMouseMoveEvent*)event;
    });

    mScene = new purrScene();

    purrObject player = mScene->newObject();
    player.getComponent<purrTransform>().setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    mPlayer = player.getUuid();

    purrObject meshObj = player.createChild();
    if (!purrMesh3D::loadModel("./assets/models/pyramid.obj", mScene, &meshObj)) return 1;

    purrObject lightObj = player.createChild();
    lightObj.addComponent<purrLightComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    lightObj.getComponent<purrTransform>().setPosition(glm::vec3(0.0f, 0.0f, -1.0f));

    purrObject obj = mScene->newObject();
    obj.addComponent<purrCameraComponent>(purrCamera());
    mScene->setCamera(obj.getUuid());

    purrRenderer3D* renderer = (purrRenderer3D*)purrRenderer::getInstance();
    renderer->setScene(mScene);
    return renderer->update();
  }

  virtual bool update(float dt) override {
    auto player_opt = mScene->getObject(mPlayer);
    if (!player_opt.has_value()) return true;
    purrObject player = player_opt.value();

    int x = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);
    int z = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);

    if (player.hasComponent<purrTransform>()) {
      purrTransform &transform = player.getComponent<purrTransform>();
      transform.setPosition(transform.getPosition() + (glm::vec3((float)x, 0.0f, (float)z) * dt));
    }

    return true;
  }

  virtual void cleanup() override {
    delete mScene;
  }
private:
  purrScene* mScene = nullptr;
  PUID mPlayer;
};

purrApp* PurrfectEngine::CreateApp() {
  return new testApp();
}