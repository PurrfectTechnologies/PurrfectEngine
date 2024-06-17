#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp: public purrApp {
public:
  testApp():
    purrApp(purrAppCreateInfo{"PurrEngineTest", purrWindowInitInfo{
      "PurrfectEngine - Test", 1920, 1080
    }, new purrRenderer3D(), purrRendererInitInfo{
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" }
    }})
  {}

  ~testApp() {

  }

  virtual void update(float dt) override {
    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int y = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    glm::vec3 rot = glm::eulerAngles(mScene->getCamera()->getTransform()->getRotation());
    rot.x -= y * dt;
    rot.y -= x * dt;
    mScene->getCamera()->getTransform()->setRotation(glm::quat(rot));
  }

  virtual void render(float dt) override {
    
  }

  virtual void resize() override {
    
  }
protected:
  virtual bool initialize() override {
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

  virtual void cleanup() override {
    delete mScene;
  }
private:
  purrScene *mScene = nullptr;
};

purrApp *PurrfectEngine::CreateApp() {
  return new testApp();
}