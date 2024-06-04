#ifndef   PURRENGINE_RENDERER_RENDERPASS_HPP_
#define   PURRENGINE_RENDERER_RENDERPASS_HPP_

#include <unordered_map>

namespace PurrfectEngine {

  struct purrPipelineCreateInfo {
    int width;
    int height;
    std::unordered_map<VkShaderStageFlagBits, const char *> shaders;
  };

  class purrPipeline {
  public:
    purrPipeline();
    ~purrPipeline();

    void initialize(purrPipelineCreateInfo createInfo);
    void cleanup();

    void begin();
    void bind();
    void end();

    static void setContext(PurrfectEngineContext *context);
  public:
    fr::frPipeline *get() const { return mPipeline; }

    purrTexture *getColor() const { return mColorTexture; }
    purrTexture *getDepth() const { return mDepthTexture; }
  private:
    fr::frPipeline *mPipeline = nullptr;

    purrTexture *mColorTexture = nullptr;
    purrTexture *mDepthTexture = nullptr;

    fr::frFramebuffer *mFramebuffer = nullptr;

    std::vector<fr::frShader*> mShaders{};
    int mWidth, mHeight;
  };

}

#endif // PURRENGINE_RENDERER_RENDERPASS_HPP_