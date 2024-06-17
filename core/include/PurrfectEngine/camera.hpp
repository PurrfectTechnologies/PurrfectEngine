#ifndef   PURRENGINE_CAMERA_HPP_
#define   PURRENGINE_CAMERA_HPP_

namespace PurrfectEngine {

  class purrCamera {
  public:
    struct Settings {
      float fov = 45.0f;
      float aspectRatio = 16.0f/9.0f;
      float nearPlane = 0.01f;
      float farPlane = 100.0f;
    };
  public:
    purrCamera(purrTransform *transform = nullptr);
    ~purrCamera();

    void setSettings(Settings settings) { mSettings = settings; }

    glm::mat4 getProjection();
    glm::mat4 getView();

    void setTransform(purrTransform *trans) { mTransform = trans; }
    purrTransform *getTransform() const { return mTransform; }
  private:
    Settings mSettings{};

    purrTransform *mTransform = nullptr;
  };

}

#endif // PURRENGINE_CAMERA_HPP_