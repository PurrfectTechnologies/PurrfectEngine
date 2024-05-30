#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  void input::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  bool input::IsKeyDown(KeyCode key) {
    return glfwGetKey(sContext->frWindow->get(), static_cast<int32_t>(key)) == GLFW_PRESS;
  }

  bool input::IsKeyUp(KeyCode key) {
    return !IsKeyDown(key);
  }
  
  bool input::IsMouseDown(MouseCode btn) {
    return glfwGetMouseButton(sContext->frWindow->get(), static_cast<int32_t>(btn)) == GLFW_PRESS;
  }
  
  glm::vec2 input::GetMousePos() {
    glm::dvec2 mouse{};
    glfwGetCursorPos(sContext->frWindow->get(), &mouse.x, &mouse.y);
    return {mouse.x,mouse.y};
  }

  void input::SetMouseMode(MouseMode mode) {
    glfwSetInputMode(sContext->frWindow->get(), GLFW_CURSOR, (int)mode);
  }

}