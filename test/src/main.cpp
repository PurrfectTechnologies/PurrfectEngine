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
    player.getComponent<purrTransform>().setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    mPlayer = player.getUuid();

    purrObject camera = player.createChild();
    camera.addComponent<purrCameraComponent>(purrCamera());
    mScene->setCamera(camera.getUuid());

    purrObject meshObj = mScene->newObject();
    meshObj.getComponent<purrTransform>().setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    if (!purrMesh3D::loadModel("./assets/models/pyramid.obj", mScene, &meshObj)) return 1;

    purrObject lightObj = mScene->newObject();
    lightObj.addComponent<purrLightComponent>(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    lightObj.getComponent<purrTransform>().setPosition(glm::vec3(0.0f, 0.0f, -1.0f));

    mRenderer = (purrRenderer3D*)purrRenderer::getInstance();
    mRenderer->setScene(mScene);
    return mRenderer->update();
  }

  virtual bool update(float dt) override {
    auto player_opt = mScene->getObject(mPlayer);
    if (!player_opt.has_value()) return true;
    purrObject player = player_opt.value();

    static constexpr float speed = 10.0f;
    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int z = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    if (player.hasComponent<purrTransform>()) {
      purrTransform &transform = player.getComponent<purrTransform>();
      glm::vec3 forward = transform.getForward() * (float)z;
      glm::vec3 right = transform.getRight() * (float)x;
      transform.setPosition(transform.getPosition() + ((forward + right) * speed * dt));
    }

    return mRenderer->update();
  }

  virtual void cleanup() override {
    delete mScene;
  }
private:
  purrRenderer3D *mRenderer = nullptr;
  purrScene *mScene = nullptr;
  PUID mPlayer;
};

purrApp* PurrfectEngine::CreateApp() {
  return new testApp();
}