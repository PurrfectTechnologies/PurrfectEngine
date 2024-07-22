#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static purrWindow *sWindow = nullptr;
  static purrEventHandler<purrEvent> *sEventHandler = new purrEventHandler<purrEvent>();

  purrEventHandler<purrEvent> *input::GetEventHandler() { return sEventHandler; }

  void input::SetWindow(purrWindow *window) {
    glfwSetWindowUserPointer(window->get(), sEventHandler);

    glfwSetKeyCallback(window->get(), [](GLFWwindow *window, int key, int scancode, int action, int mods) {
      purrEventHandler<purrEvent> *events = (purrEventHandler<purrEvent>*)glfwGetWindowUserPointer(window);
      events->fire(new purrKeyEvent(key, action));
    });

    glfwSetMouseButtonCallback(window->get(), [](GLFWwindow *window, int button, int action, int mods) {
      purrEventHandler<purrEvent> *events = (purrEventHandler<purrEvent>*)glfwGetWindowUserPointer(window);
      events->fire(new purrMouseBtnEvent(button, action));
    });

    glfwSetCursorPosCallback(window->get(), [](GLFWwindow *window, double x, double y) {
      purrEventHandler<purrEvent> *events = (purrEventHandler<purrEvent>*)glfwGetWindowUserPointer(window);
      events->fire(new purrMouseMoveEvent(glm::dvec2(x, y)));
    });

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

  purrKeyEvent::purrKeyEvent(input::KeyCode key, input::ButtonAction action):
    purrEvent("keyEvent"), mKey(key), mAction(action)
  {}

  purrMouseBtnEvent::purrMouseBtnEvent(input::MouseCode code, input::ButtonAction action):
    purrEvent("mouseButtonEvent"), mCode(code), mAction(action)
  {}

  purrMouseMoveEvent::purrMouseMoveEvent(glm::dvec2 pos):
    purrEvent("mouseMoveEvent"), mPos(pos)
  {}

}