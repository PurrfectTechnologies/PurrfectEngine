#include "PurrfectEngine/PurrfectEngine.hpp"

#include <stdio.h>

namespace PurrfectEngine {

  purrApp::purrApp(purrAppCreateInfo createInfo):
    mCreateInfo(createInfo)
  {
    assert(mCreateInfo.renderer && "Renderer is required!");
    sInstance = this;
  }

  purrApp::~purrApp() {

  }

  bool purrApp::init() {
    mWindow = new purrWindow();

    return mWindow->initialize(mCreateInfo.windowInitInfo) && mCreateInfo.renderer->initialize(mWindow, mCreateInfo.rendererInitInfo) && initialize();
  }

  void purrApp::run() {
    float lastTime = 0;
    while (!mWindow->shouldClose()) {
      float time = (float)glfwGetTime();
      float deltaTime = time - lastTime;
      lastTime = time;

      glfwPollEvents();

      update(deltaTime);

      if (!mCreateInfo.renderer->render()) break;
    }

    mCreateInfo.renderer->waitIdle();
    cleanup();
    mCreateInfo.renderer->cleanup();
    delete mCreateInfo.renderer;
    delete mWindow;
  }

  void purrApp::SetScene(purrScene *scene) {
    mScene = scene;
  }

  glm::ivec2 purrApp::GetSize() {
    return mCreateInfo.renderer->getSwapchainSize();
  }

}