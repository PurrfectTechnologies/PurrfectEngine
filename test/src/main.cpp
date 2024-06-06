#include <iostream>

#define PURRENGINE_MAIN
#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;

class testApp: public purrApp {
public:
  testApp():
    purrApp("PurrfectEngine - Test")
  {}

  ~testApp() {

  }

  virtual void update(float dt) override {
    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int z = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    glm::vec3 pos = mScene->getCamera()->getTransform()->getPosition();
    pos.x += x * dt;
    pos.z += z * dt;
    mScene->getCamera()->getTransform()->setPosition(pos);
  }

  virtual void render(float dt) override {
    renderer::beginScenePass();
    mScenePipeline->bind();
    // renderer::renderScene(mScenePipeline);
    renderer::endScenePass();
  }

  virtual void resize() override {
    glm::ivec2 size = GetSize();
    recreateSceneObjects(size.x, size.y);
  }
protected:
  virtual bool initialize() override {
    mScene = new purrScene();
    { // Initialize object
      purrObject *object = new purrObject();
      purrMesh *mesh = new purrMesh();
      mesh->initialize("../test/models/ico.obj");
      if (!mesh->isValid()) return false;
      object->addComponent(new purrMeshComp(mesh));
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

    glm::ivec2 size = GetSize();
    createSceneObjects(size.x, size.y);

    { // Initialize skybox
      const char *const filepath = "../test/textures/skybox.hdr";
      purrTexture *texture = new purrTexture(0, 0, VK_FORMAT_R32G32B32A32_SFLOAT);
      if (!texture->initializeHdr(filepath, 4)) {
        printf("Failed to load skybox texture (%s)\n", filepath);
        return false;
      }
      mSkybox = new purrSkybox();
      if (!mSkybox->initialize(texture, 2048, 2048)) {
        printf("Failed to init skybox\n");
        return false;
      }
      delete texture;
    }

    return true;
  }

  virtual void cleanup() override {
    delete mScene;
    delete mSkybox;
    cleanupSceneObjects();
  }
private:
  void createSceneObjects(int width, int height) {
    mScenePipeline = new purrPipeline();
    mScenePipeline->initialize({
      width, height,
      { {VK_SHADER_STAGE_VERTEX_BIT, "../test/shaders/vert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "../test/shaders/PBR/pbr.spv"} }
    });
    
    mSceneRenderTarget = new purrRenderTarget();
    mSceneRenderTarget->initialize(glm::ivec2(width, height));

    renderer::setSceneTarget(mSceneRenderTarget);
  }

  void cleanupSceneObjects() {
    delete mScenePipeline;
    delete mSceneRenderTarget;
  }

  void recreateSceneObjects(int width, int height) {
    cleanupSceneObjects();
    createSceneObjects(width, height);
  }
private:
  purrScene    *mScene = nullptr;
  purrPipeline *mScenePipeline = nullptr;
  purrRenderTarget *mSceneRenderTarget = nullptr;

  purrSkybox *mSkybox = nullptr;
};

purrApp *PurrfectEngine::CreateApp() {
  return new testApp();
}