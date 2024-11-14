#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp : public purrApp {
public:
  testApp() :
    purrApp(purrAppCreateInfo{ "PurrEngineTest" }, {
      new purrAppRendererExt(new purrRenderer3D(), purrWindowInitInfo{ "PurrfectEngine - Test", 1920, 1080 }),
      new purrAppAudioExt()
    })
  {}

  ~testApp() {
    cleanup();
  }

protected:
  virtual bool initialize() override {
    input::GetEventHandler()->on("keyEvent", [this](purrEvent* event) {
      purrKeyEvent *keyEvent = (purrKeyEvent*)event;
      if (keyEvent->getKey() == input::key::Escape && keyEvent->getAction() == input::button::Pressed) mPaused = !mPaused;
      input::SetMouseMode(mPaused?input::MouseMode::Normal:input::MouseMode::Disabled);
    });

    // input::GetEventHandler()->on("mouseButtonEvent", [](purrEvent* event) {
    //   purrMouseBtnEvent *mouseEvent = (purrMouseBtnEvent*)event;
    // });

    input::GetEventHandler()->on("mouseMoveEvent", [](purrEvent* event) {
      purrMouseMoveEvent *mouseEvent = (purrMouseMoveEvent*)event;
    });

    input::SetMouseMode(input::MouseMode::Disabled);

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
    if (mPaused) return true;

    auto player_opt = mScene->getObject(mPlayer);
    if (!player_opt.has_value()) return true;
    purrObject player = player_opt.value();

    static constexpr float speed = 100.0f;
    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int z = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    if (player.hasComponent<purrTransform>()) {
      purrTransform &transform = player.getComponent<purrTransform>();
      glm::vec3 forward = transform.getForward() * (float)z;
      glm::vec3 right = transform.getRight() * (float)x;
      if (glm::length(forward + right) > 0) transform.setPosition(transform.getPosition() + (glm::normalize(forward + right) * speed * dt));

      static constexpr float sensitivity = 100.0;
      glm::dvec2 mouseDeltaD = input::GetMouseDelta();
      glm::vec2 mouseDelta = glm::vec2((float)mouseDeltaD.x, (float)mouseDeltaD.y) * sensitivity * dt;
      mRotationX -= mouseDelta.y;
      mRotationX  = glm::clamp(mRotationX, -90.0f, 90.0f);

      transform.setRotation(glm::angleAxis(mouseDelta.x, glm::vec3(0.0f, 1.0f, 0.0f)) * transform.getRotation());

      auto cameraOpt = mScene->getCamera();
      if (cameraOpt.has_value()) {
        purrTransform &cameraTransform = cameraOpt.value().getComponent<purrTransform>();
        cameraTransform.setRotation(glm::quat(glm::vec3(glm::radians(mRotationX), 0.0f, 0.0f)));
      }
    }

    return mRenderer->update();
  }

  virtual void cleanup() override {
    delete mScene;
  }
private:
  bool mPaused = false;
  float mRotationX = 0.0f;
private:
  purrRenderer3D *mRenderer = nullptr;
  purrScene *mScene = nullptr;
  PUID mPlayer;
};

purrApp* PurrfectEngine::CreateApp() {
  return new testApp();
}