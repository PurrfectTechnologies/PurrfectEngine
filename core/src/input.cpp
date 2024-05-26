#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  void Input::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  bool Input::IsKeyDown(KeyCode key) {
    return glfwGetKey(sContext->frWindow->get(), static_cast<int32_t>(key)) == GLFW_PRESS;
  }

  bool Input::IsKeyUp(KeyCode key) {
    return !IsKeyDown(key);
  }
  
  bool Input::IsMouseDown(MouseCode btn) {
    return glfwGetMouseButton(sContext->frWindow->get(), static_cast<int32_t>(btn)) == GLFW_PRESS;
  }
  
  glm::vec2 Input::GetMousePos() {
    glm::dvec2 mouse{};
    glfwGetCursorPos(sContext->frWindow->get(), &mouse.x, &mouse.y);
    return {mouse.x,mouse.y};
  }

  void Input::SetMouseMode(MouseMode mode) {
    glfwSetInputMode(sContext->frWindow->get(), GLFW_CURSOR, (int)mode);
  }

}