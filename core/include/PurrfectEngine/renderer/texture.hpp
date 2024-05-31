#ifndef   PURRENGINE_RENDERER_TEXTURE_HPP_
#define   PURRENGINE_RENDERER_TEXTURE_HPP_

namespace PurrfectEngine {

  class purrSampler {
    friend class purrTexture;
  public:
    purrSampler();
    ~purrSampler();

    void initialize(fr::frSampler::frSamplerInfo info);
    void cleanup();

    static purrSampler *getDefault();

    static void cleanupAll();
  private:
    fr::frSampler *mSampler = nullptr;
  };

  class purrTexture {
    friend class purrPipeline;
  public:
    purrTexture(int width, int height, VkFormat format);
    ~purrTexture();

    // if (!sampler) mDescriptor = nullptr;
    void initialize(purrSampler *sampler = purrSampler::getDefault(), bool mipmaps = true, bool color = true);
    void cleanup();
    void resize(int width, int height);

    void setPixels(std::vector<uint8_t> pixels);
    void setPixels(uint8_t *pixels, size_t size);
    void getPixels(std::vector<uint8_t>& pixels, size_t& size) const;

    static void setContext(PurrfectEngineContext *context);
  public:
    fr::frImage *getImage() const { return mImage; }

    fr::frDescriptor *getDescriptor() const { return mDescriptor; }
  private:
    int mWidth = 0, mHeight = 0;
    VkFormat mFormat = VK_FORMAT_UNDEFINED;
    bool mMipmaps = true;
    bool mColor = true;
    VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;

    fr::frImage *mImage = nullptr;
    purrSampler *mSampler = nullptr;
    fr::frDescriptor *mDescriptor = nullptr;
  };

}

#endif // PURRENGINE_RENDERER_TEXTURE_HPP_