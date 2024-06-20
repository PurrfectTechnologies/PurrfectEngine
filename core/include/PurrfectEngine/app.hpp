#ifndef   PURRENGINE_APP_HPP_
#define   PURRENGINE_APP_HPP_

namespace PurrfectEngine {
  struct purrAppCreateInfo {
    const char *applicationName;
  };

  class purrApp {
  public:
    purrApp(purrAppCreateInfo createInfo);
    ~purrApp();

    bool init();
    void run();
  
    virtual bool initialize() = 0;
    virtual bool update(float dt) = 0;
    virtual void cleanup() = 0;
  public:
    const char *getAppName() const { return mCreateInfo.applicationName; }
  protected:
    void SetScene(purrScene *scene);
  protected:
    bool mRunning = true;
  public:
    static purrApp *get() { return sInstance; }
  private:
    purrScene *mScene = nullptr;
  private:
    purrAppCreateInfo mCreateInfo;
    inline static purrApp *sInstance = nullptr;
  };

  purrApp *CreateApp();
}

#endif // PURRENGINE_APP_HPP_