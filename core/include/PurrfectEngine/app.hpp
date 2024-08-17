#ifndef   PURRENGINE_APP_HPP_
#define   PURRENGINE_APP_HPP_

namespace PurrfectEngine {
  struct purrAppCreateInfo {
    const char *applicationName;
  };

  class purrAppExt {
  public:
    virtual ~purrAppExt() = default;

    virtual bool initialize() = 0;
    virtual bool preUpdate()  = 0;
    virtual bool update()     = 0;
    virtual void cleanup()    = 0;
  private:
  };

  class purrAppRendererExt: public purrAppExt {
  public:
    purrAppRendererExt(purrRenderer *renderer, purrWindowInitInfo windowInfo, purrRendererInitInfo rendererInfo);
    ~purrAppRendererExt();

    virtual bool initialize() override;
    virtual bool preUpdate()  override;
    virtual bool update()     override;
    virtual void cleanup()    override;
  private:
    purrWindowInitInfo mWindowInfo;
    purrRendererInitInfo mRendererInfo;
  private:
    purrWindow *mWindow = nullptr;
    purrRenderer *mRenderer = nullptr;
  };

  class purrApp: public purrExtendable<purrAppExt> {
  public:
    purrApp(purrAppCreateInfo createInfo, std::vector<purrAppExt*> extensions);
    ~purrApp();

    bool init();
    void run();
  
    virtual bool initialize() = 0;
    virtual bool update(float dt) = 0;
    virtual void cleanup() = 0;
  public:
    const char *getAppName() const { return mCreateInfo.applicationName; }
  protected:
    bool mRunning = true;
  public:
    static purrApp *get() { return sInstance; }
  private:
    purrAppCreateInfo mCreateInfo;
    inline static purrApp *sInstance = nullptr;
  };

  purrApp *CreateApp();
}

#endif // PURRENGINE_APP_HPP_