#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp: public purrApp {
public:
  testApp():
    purrApp(purrAppCreateInfo{"PurrEngineTest"})
  {}

  ~testApp() {

  }
protected:
  virtual bool initialize() override {
    mWindow = new purrWindow();
    if (!mWindow->initialize(purrWindowInitInfo{
      "PurrfectEngine - Test", 1920, 1080
    })) return false;
    input::SetWindow(mWindow);

    mRenderer = new purrRenderer3D();
    if (!mRenderer->initialize(mWindow, purrRendererInitInfo{
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" }
    })) return false;

    mScene = new purrScene();
    { // Initialize object
      purrObject *object = new purrObject();
      // purrMesh *mesh = new purrMesh();
      // mesh->initialize("../test/models/ico.obj");
      // if (!mesh->isValid()) return false;
      // object->addComponent(new purrMeshComp(mesh));
      object->getTransform()->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
      mScene->addObject(object);
    }

    { // Initialize camera
      purrObject *object = new purrObject(new purrTransform(glm::vec3(0.0f, 0.0f, -5.0f)));
      object->addComponent(new purrCameraComp(new purrCamera()));
      mScene->addObject(object);
      mScene->setCamera(object);
    }

    SetScene(mScene);

    return true;
  }

  virtual bool update(float dt) override {
    glfwPollEvents();

    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int y = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    glm::vec3 rot = glm::eulerAngles(mScene->getCamera()->getTransform()->getRotation());
    rot.x -= y * dt;
    rot.y -= x * dt;
    mScene->getCamera()->getTransform()->setRotation(glm::quat(rot));

    bool r = render(dt);
    mRunning = !mWindow->shouldClose();
    return r;
  }

  bool render(float dt) {
    return mRenderer->render();
  }

  virtual void cleanup() override {
    mRenderer->waitIdle();
    mRenderer->cleanup();
    delete mRenderer;
    delete mWindow;
    delete mScene;
  }
private:
  purrWindow *mWindow = nullptr;
  purrRenderer3D *mRenderer = nullptr;
  purrScene *mScene = nullptr;
};

purrApp *PurrfectEngine::CreateApp() {
  return new testApp();
}