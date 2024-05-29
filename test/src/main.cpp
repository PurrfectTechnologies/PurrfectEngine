#include <iostream>

#include <PurrfectEngine/PurrfectEngine.hpp>

using namespace PurrfectEngine;
namespace renderer = PurrfectEngine::renderer;
namespace input = PurrfectEngine::Input;

purrPipeline* scenePipeline = nullptr;
purrTexture* sceneRenderTarget = nullptr;
purrSampler* sceneSampler = nullptr;

void createSceneObjects(int width, int height) {
  sceneRenderTarget = new PurrfectEngine::purrTexture(width, height, VK_FORMAT_R16G16B16A16_SFLOAT);
  sceneRenderTarget->initialize(sceneSampler, false);
  PurrfectEngine::purrPipelineCreateInfo pipelineInfo = {
    width, height,
    { {VK_SHADER_STAGE_VERTEX_BIT, "../shaders/vert.spv"}, {VK_SHADER_STAGE_FRAGMENT_BIT, "../shaders/frag.spv"} },
    sceneRenderTarget
  };
  scenePipeline = new PurrfectEngine::purrPipeline(pipelineInfo);
  scenePipeline->initialize();
  renderer::setScenePipeline(scenePipeline);
}

void cleanupSceneObjects() {
  delete sceneRenderTarget;
  delete scenePipeline;
}

void recreateSceneObjects(int width, int height) {
  cleanupSceneObjects();
  createSceneObjects(width, height);
}

int main(int argc, char **argv) {
  PurrfectEngine::PurrfectEngineContext *context = new PurrfectEngine::PurrfectEngineContext();

  try {
  input::setContext(context);
  renderer::setContext(context);
  renderer::setVSync(true);
  renderer::initialize("PurrfectEngine - Test", 1920, 1080);

  purrScene *scene = new purrScene();
  { // Initialize object
    purrObject *object = new purrObject();
    purrMesh *mesh = new purrMesh();
    mesh->initialize("../models/ico.obj");
    if (!mesh->isValid()) return 1;
    object->addComponent(new purrMeshComp(mesh));
    object->getTransform()->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    scene->addObject(object);
  }

  { // Initialize camera
    purrObject *object = new purrObject(new purrTransform(glm::vec3(0.0f, 0.0f, -5.0f)));
    object->addComponent(new purrCameraComp(new purrCamera()));
    scene->addObject(object);
    scene->setCamera(object);
  }
  renderer::setScene(scene);

  sceneSampler = new purrSampler();
  sceneSampler->initialize(fr::frSampler::frSamplerInfo{});

  int width = 0, height = 0;
  renderer::getSwapchainSize(&width, &height);
  createSceneObjects(width, height);

  bool escapePressed = false;
  float lastTime = 0;
  while (!renderer::shouldClose()) {
    float time = (float)glfwGetTime();
    float deltaTime = time - lastTime;
    lastTime = time;

    glfwPollEvents();

    int x = input::IsKeyDown(input::key::D) - input::IsKeyDown(input::key::A);
    int z = input::IsKeyDown(input::key::W) - input::IsKeyDown(input::key::S);

    glm::vec3 pos = scene->getCamera()->getTransform()->getPosition();
    pos.x += x * deltaTime;
    pos.z += z * deltaTime;
    scene->getCamera()->getTransform()->setPosition(pos);

    if (!renderer::renderBegin()) {
      renderer::getSwapchainSize(&width, &height);
      recreateSceneObjects(width, height);
      continue;
    }

    renderer::updateCamera();
    renderer::updateTransforms();
    scenePipeline->begin({{{0.0f, 0.0f, 0.0f, 1.0f}}});
    renderer::renderScene(scenePipeline);
    scenePipeline->end();

    renderer::render();
    if (!renderer::present()) {
      renderer::getSwapchainSize(&width, &height);
      recreateSceneObjects(width, height);
    }
  }
  
  renderer::waitIdle();
  delete scene;
  delete sceneSampler;
  cleanupSceneObjects();
  renderer::cleanup();
  } catch (fr::frVulkanException &ex) {
    fprintf(stderr, "Vulkan exception caught: %s\n", ex.what());
  }

  delete context;

  return 0;
}