#ifndef   PURRENGINE_RENDERER_SKYBOX_HPP_
#define   PURRENGINE_RENDERER_SKYBOX_HPP_

namespace PurrfectEngine {

  class purrSkybox {
  public:
    purrSkybox();
    ~purrSkybox();

    bool initialize(purrTexture *texture, int width, int height);
    bool initialize(purrCubemap *cubemap);
    void cleanup();

    void bind(VkCommandBuffer cmdBuf, fr::frPipeline *pipeline);

    static void setContext(PurrfectEngineContext *context);
  private:
    purrCubemap *textureToCubemap(purrTexture *texture, int width, int height);
  private:
    purrCubemap *mSkyboxCubemap = nullptr;
    fr::frDescriptor *mSkyboxDesc = nullptr;
  };

}

#endif // PURRENGINE_RENDERER_SKYBOX_HPP_