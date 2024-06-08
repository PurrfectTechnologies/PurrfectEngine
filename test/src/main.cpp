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
    int y = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    glm::vec3 rot = glm::eulerAngles(mScene->getCamera()->getTransform()->getRotation());
    rot.x -= y * dt;
    rot.y -= x * dt;
    mScene->getCamera()->getTransform()->setRotation(glm::quat(rot));
  }

  virtual void render(float dt) override {
    renderer::beginScenePass();
    mSkybox->render(mSize.x, mSize.y);
    mScenePipeline->bind();
    mSkybox->bind(mScenePipeline);
    renderer::renderScene(mScenePipeline, mRoughness, mNormal, mMetalic);
    renderer::endScenePass();
  }

  virtual void resize() override {
    mSize = GetSize();
    recreateSceneObjects();
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

    mSize = GetSize();
    createSceneObjects();

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

    { // Load textures
      // mAlbedo = new purrTexture(0, 0, VK_FORMAT_B8G8R8A8_SRGB);
      // if (!mAlbedo->initialize("../test/textures/albedo.png")) return false;
      mMetalic = new purrTexture(0, 0, VK_FORMAT_B8G8R8A8_SRGB);
      if (!mMetalic->initialize("../test/textures/metallic.png")) return false;
      mNormal = new purrTexture(0, 0, VK_FORMAT_B8G8R8A8_SRGB);
      if (!mNormal->initialize("../test/textures/normal.png")) return false;
      mRoughness = new purrTexture(0, 0, VK_FORMAT_B8G8R8A8_SRGB);
      if (!mRoughness->initialize("../test/textures/roughness.png")) return false;
    }

    return true;
  }

  virtual void cleanup() override {
    // delete mAlbedo;
    delete mMetalic;
    delete mNormal;
    delete mRoughness;

    delete mScene;
    delete mSkybox;
    cleanupSceneObjects();
  }
private:
  void createSceneObjects() {
    mScenePipeline = new purrPipeline();
    mScenePipeline->initialize({
      mSize.x, mSize.y,
      { {VK_SHADER_STAGE_VERTEX_BIT, "../test/shaders/vert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "../test/shaders/PBR/pbr.spv"} }
    });
    
    mSceneRenderTarget = new purrRenderTarget();
    mSceneRenderTarget->initialize(mSize);

    renderer::setSceneTarget(mSceneRenderTarget);
  }

  void cleanupSceneObjects() {
    delete mScenePipeline;
    delete mSceneRenderTarget;
  }

  void recreateSceneObjects() {
    cleanupSceneObjects();
    createSceneObjects();
  }
private:
  glm::ivec2 mSize = {};

  purrScene    *mScene = nullptr;
  purrPipeline *mScenePipeline = nullptr;
  purrRenderTarget *mSceneRenderTarget = nullptr;
  
  // purrTexture *mAlbedo = nullptr;
  purrTexture *mMetalic = nullptr;
  purrTexture *mNormal = nullptr;
  purrTexture *mRoughness = nullptr;

  purrSkybox *mSkybox = nullptr;
};

purrApp *PurrfectEngine::CreateApp() {
  return new testApp();
}