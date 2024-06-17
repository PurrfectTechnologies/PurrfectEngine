#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  bool input::IsKeyDown(KeyCode key) {
    return glfwGetKey(purrApp::get()->getWindow()->get(), static_cast<int32_t>(key)) == GLFW_PRESS;
  }

  bool input::IsKeyUp(KeyCode key) {
    return !IsKeyDown(key);
  }
  
  bool input::IsMouseDown(MouseCode btn) {
    return glfwGetMouseButton(purrApp::get()->getWindow()->get(), static_cast<int32_t>(btn)) == GLFW_PRESS;
  }
  
  glm::vec2 input::GetMousePos() {
    glm::dvec2 mouse{};
    glfwGetCursorPos(purrApp::get()->getWindow()->get(), &mouse.x, &mouse.y);
    return {mouse.x,mouse.y};
  }

  void input::SetMouseMode(MouseMode mode) {
    glfwSetInputMode(purrApp::get()->getWindow()->get(), GLFW_CURSOR, (int)mode);
  }

}