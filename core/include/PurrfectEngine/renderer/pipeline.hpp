#ifndef   PURRENGINE_RENDERER_RENDERPASS_HPP_
#define   PURRENGINE_RENDERER_RENDERPASS_HPP_

#include <unordered_map>

namespace PurrfectEngine {

  struct purrPipelineCreateInfo {
    int width, height;
    std::unordered_map<VkShaderStageFlagBits, const char *> shaders;
  };

  class purrPipeline {
  public:
    purrPipeline();
    ~purrPipeline();

    void initialize(purrPipelineCreateInfo createInfo);
    void cleanup();

    void bind();

    static void setContext(PurrfectEngineContext *context);
  public:
    fr::frPipeline *get() const { return mPipeline; }
  private:
    fr::frPipeline *mPipeline = nullptr;

    std::vector<fr::frShader*> mShaders{};
  };

}

#endif // PURRENGINE_RENDERER_RENDERPASS_HPP_