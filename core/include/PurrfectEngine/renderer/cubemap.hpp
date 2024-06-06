#ifndef   PURRENGINE_RENDERER_CUBEMAP_HPP_
#define   PURRENGINE_RENDERER_CUBEMAP_HPP_

namespace PurrfectEngine {

  class purrCubemap {
    friend class purrSkybox;
  public:
    purrCubemap();
    ~purrCubemap();

    bool initialize(std::array<purrTexture *,6> textures, VkFormat format, purrSampler *sampler = purrSampler::getDefault());
    void cleanup();

    static void setContext(PurrfectEngineContext *context);
  public:
    fr::frImage *getImage() const { return mImage; }

    fr::frDescriptor *getDescriptor() const { return mDescriptor; }
  private:
    int mWidth = 0, mHeight = 0;
    VkFormat mFormat = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;

    fr::frImage *mImage = nullptr;
    purrSampler *mSampler = nullptr;
    fr::frDescriptor *mDescriptor = nullptr;
  };

}

#endif // PURRENGINE_RENDERER_CUBEMAP_HPP_