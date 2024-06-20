#include "PurrfectEngine/PurrfectEngine.hpp"

#include <stdio.h>

namespace PurrfectEngine {

  purrAppRendererExt::purrAppRendererExt(purrRenderer *renderer, purrWindowInitInfo windowInfo, purrRendererInitInfo rendererInfo):
    mRenderer(renderer), mWindowInfo(windowInfo), mRendererInfo(rendererInfo)
  {}

  bool purrAppRendererExt::initialize() {
    mWindow = new purrWindow();
    if (!mWindow->initialize(purrWindowInitInfo{
      "PurrfectEngine - Test", 1920, 1080
    })) return false;
    input::SetWindow(mWindow);

    mRenderer = new purrRenderer3D();
    return mRenderer->initialize(mWindow, purrRendererInitInfo{
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" }
    });
  }

  bool purrAppRendererExt::preUpdate() {
    glfwPollEvents();

    return !mWindow->shouldClose();
  }

  bool purrAppRendererExt::update() {
    return mRenderer->render();
  }

  void purrAppRendererExt::cleanup() {
    mRenderer->waitIdle();
    mRenderer->cleanup();
    delete mRenderer;
    delete mWindow;
  }

  purrApp::purrApp(purrAppCreateInfo createInfo, std::vector<purrAppExt*> extensions):
    purrExtendable<purrAppExt>(extensions), mCreateInfo(createInfo)
  {
    sInstance = this;
  }

  purrApp::~purrApp() {

  }

  bool purrApp::init() {
    return extsInitialize() && initialize();
  }

  void purrApp::run() {
    float lastTime = 0;
    while (mRunning) {
      float time = (float)glfwGetTime();
      float deltaTime = time - lastTime;
      lastTime = time;

      if (!extsPreUpdate() || !update(deltaTime) || !extsUpdate()) break;
    }

    extsCleanup();
    cleanup();
  }

  void purrApp::SetScene(purrScene *scene) {
    mScene = scene;
  }

}