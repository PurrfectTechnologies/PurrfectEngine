#ifndef   PURRENGINE_RENDERER_RENDERTARGET_HPP_
#define   PURRENGINE_RENDERER_RENDERTARGET_HPP_

namespace PurrfectEngine {

  class purrRenderTarget {
  public:
    purrRenderTarget();
    ~purrRenderTarget();

    void initialize(glm::ivec2 size);
    void cleanup();

    void begin(VkCommandBuffer cmdBuf);
    void end(VkCommandBuffer cmdBuf);

    static void setContext(PurrfectEngineContext *context);
  public:
    purrTexture *getColorTarget() const { return mColorTarget; }
    purrTexture *getDepthTarget() const { return mDepthTarget; }
  private:
    purrTexture *mColorTarget = nullptr;
    purrTexture *mDepthTarget = nullptr;
    fr::frFramebuffer *mFramebuffer = nullptr;
    glm::ivec2 mSize{};

    VkCommandBuffer mCmdBuf = VK_NULL_HANDLE;
  };

}

#endif // PURRENGINE_RENDERER_RENDERTARGET_HPP_