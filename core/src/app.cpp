#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  purrApp::purrApp(const char *windowName):
    mWindowName(windowName)
  { sInstance = this; }

  purrApp::~purrApp() {
    try {
    renderer::cleanup();
    } catch (fr::frVulkanException &ex) {
      fprintf(stderr, "Vulkan exception caught: %s\n", ex.what());
    }
  }

  bool purrApp::init() {
    sContext = new PurrfectEngineContext();
    input::setContext(sContext);

    try {
    renderer::setContext(sContext);
    renderer::initialize(mWindowName, 1920, 1080);
    } catch (fr::frVulkanException &ex) {
      fprintf(stderr, "Vulkan exception caught: %s\n", ex.what());
      return false;
    }

    return initialize();
  }

  void purrApp::run() {
    float lastTime = 0;
    try {
    while (!renderer::shouldClose()) {
      float time = (float)glfwGetTime();
      float deltaTime = time - lastTime;
      lastTime = time;

      glfwPollEvents();

      update(deltaTime);

      if (!renderer::renderBegin()) {
        resize();
        continue;
      }

      renderer::updateCamera();
      renderer::updateTransforms();
      renderer::updateLights();
      render(deltaTime);

      renderer::render();
      if (!renderer::present()) {
        resize();
      }
    }
    } catch (fr::frVulkanException &ex) {
      fprintf(stderr, "Vulkan exception caught: %s\n", ex.what());
    }

    renderer::waitIdle();
  }

  void purrApp::SetScene(purrScene *scene) {
    renderer::setScene(scene);
  }

  glm::ivec2 purrApp::GetSize() {
    int w, h;
    renderer::getSwapchainSize(&w, &h);
    return glm::ivec2(w, h);
  }

}