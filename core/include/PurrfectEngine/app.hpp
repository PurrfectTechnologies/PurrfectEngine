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
  public:
    static purrApp *get() { return sInstance; }
  private:
    const char *mWindowName = nullptr;
    inline static purrApp *sInstance = nullptr;
  };

  purrApp *CreateApp();

}

#endif // PURRENGINE_APP_HPP_