#include "PurrfectEngine/PurrfectEngine.hpp"

#include <stdio.h>

namespace PurrfectEngine {

  purrApp::purrApp(purrAppCreateInfo createInfo):
    mCreateInfo(createInfo)
  {
    sInstance = this;
  }

  purrApp::~purrApp() {

  }

  bool purrApp::init() {
    return initialize();
  }

  void purrApp::run() {
    float lastTime = 0;
    while (mRunning) {
      float time = (float)glfwGetTime();
      float deltaTime = time - lastTime;
      lastTime = time;

      if (!update(deltaTime)) break;
    }

    cleanup();
  }

  void purrApp::SetScene(purrScene *scene) {
    mScene = scene;
  }

}