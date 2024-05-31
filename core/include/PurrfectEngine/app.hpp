#ifndef   PURRENGINE_APP_HPP_
#define   PURRENGINE_APP_HPP_

namespace PurrfectEngine {

  class purrApp {
  public:
    purrApp(const char *windowName);
    ~purrApp();

    bool init();
    void run();

    virtual void update(float dt) = 0;
    virtual void render(float dt) = 0;
    virtual void resize() = 0;
  
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
  protected:
    void SetScene(purrScene *scene);
    void SetFinalPipeline(purrPipeline *pipeline);
    glm::ivec2 GetSize();

    void setAssetDir(const char*);
    const char* getAssetDir() const;

    void setProjDir(const char*);
    const char* getProjDir() const;

  public:
    static purrApp *get() { return sInstance; }
  private:
    char* mAssetDir;
    char* mProjDir;

    const char *mWindowName = nullptr;
    inline static purrApp *sInstance = nullptr;
  };

  purrApp *CreateApp();

}

#endif // PURRENGINE_APP_HPP_