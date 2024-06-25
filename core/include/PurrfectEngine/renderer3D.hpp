#ifndef   PURRENGINE_RENDERER3D_HPP_
#define   PURRENGINE_RENDERER3D_HPP_

namespace PurrfectEngine {

  class purrRenderer3D: public purrRenderer {
  public:
    purrRenderer3D();
    ~purrRenderer3D();
  private:
    bool createResources();
  private:
    virtual bool initialize_() override;
    virtual bool resize_()     override;
    virtual bool render_()     override;
    virtual void cleanup_()    override;

    virtual VkFormat getRenderTargetFormat() override;
    virtual VkFormat getHdrFormat() override;
    virtual VkFormat getFormat() override;
    virtual VkSampleCountFlagBits getSampleCount() override;
    virtual purrRenderTarget *getRenderTarget() override;
  private:
    purrSampler      *mSampler = nullptr;
    purrRenderTarget *mRenderTarget = nullptr;
    purrPipeline     *mPipeline = nullptr;
  };

}

#endif // PURRENGINE_RENDERER3D_HPP_