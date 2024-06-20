#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static purrWindow *sWindow = nullptr;

  void input::SetWindow(purrWindow *window) {
    sWindow = window;
  }

  bool input::IsKeyDown(KeyCode key) {
    return glfwGetKey(sWindow->get(), static_cast<int32_t>(key)) == GLFW_PRESS;
  }

  bool input::IsKeyUp(KeyCode key) {
    return !IsKeyDown(key);
  }
  
  bool input::IsMouseDown(MouseCode btn) {
    return glfwGetMouseButton(sWindow->get(), static_cast<int32_t>(btn)) == GLFW_PRESS;
  }
  
  glm::vec2 input::GetMousePos() {
    glm::dvec2 mouse{};
    glfwGetCursorPos(sWindow->get(), &mouse.x, &mouse.y);
    return {mouse.x,mouse.y};
  }

  void input::SetMouseMode(MouseMode mode) {
    glfwSetInputMode(sWindow->get(), GLFW_CURSOR, (int)mode);
  }

}