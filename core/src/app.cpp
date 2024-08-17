#include "PurrfectEngine/PurrfectEngine.hpp"

#include <stdio.h>

namespace PurrfectEngine {

  purrAppRendererExt::purrAppRendererExt(purrRenderer *renderer, purrWindowInitInfo windowInfo, purrRendererInitInfo rendererInfo):
    mRenderer(renderer), mWindowInfo(windowInfo), mRendererInfo(rendererInfo)
  {}

  purrAppRendererExt::~purrAppRendererExt() {
    delete mRenderer;
    delete mWindow;
  }

  bool purrAppRendererExt::initialize() {
    mWindow = new purrWindow();
    if (!mWindow->initialize(purrWindowInitInfo{
      "PurrfectEngine - Test", 1920, 1080
    })) return false;
    input::SetWindow(mWindow);

    mRenderer = new purrRenderer3D();
    return mRenderer->initialize(mWindow);
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
    for (purrAppExt *ext: getExtensions()) delete ext;
  }

}