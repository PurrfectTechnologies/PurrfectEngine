#ifndef   PURRENGINE_APP_HPP_
#define   PURRENGINE_APP_HPP_

namespace PurrfectEngine {
  struct purrAppCreateInfo {
    const char *applicationName;
    purrWindowInitInfo windowInitInfo;
    purrRenderer *renderer;
    purrRendererInitInfo rendererInitInfo;
  };

  class purrApp {
  public:
    purrApp(purrAppCreateInfo createInfo);
    ~purrApp();

    bool init();
    void run();

    virtual void update(float dt) = 0;
    virtual void render(float dt) = 0;
    virtual void resize() = 0;
  
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
  public:
    purrWindow *getWindow() const { return mWindow; }
    const char *getAppName() const { return mCreateInfo.applicationName; }
  protected:
    void SetScene(purrScene *scene);
    glm::ivec2 GetSize();
  public:
    static purrApp *get() { return sInstance; }
  private:
    purrScene *mScene = nullptr;
  private:
    purrWindow *mWindow = nullptr;
    purrAppCreateInfo mCreateInfo;
    inline static purrApp *sInstance = nullptr;
  };

  purrApp *CreateApp();
}

#endif // PURRENGINE_APP_HPP_