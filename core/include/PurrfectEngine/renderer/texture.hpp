#ifndef   PURRENGINE_RENDERER_TEXTURE_HPP_
#define   PURRENGINE_RENDERER_TEXTURE_HPP_

namespace PurrfectEngine {

  class purrSampler {
    friend class purrTexture;
    friend class purrCubemap;
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
    purrTexture(int width, int height, VkFormat format, fr::frCommands *commands = nullptr);
    ~purrTexture();

    // if (!sampler) mDescriptor = nullptr;
    bool initialize(const char *filename = nullptr, purrSampler *sampler = purrSampler::getDefault(), bool mipmaps = true, bool color = true);
    bool initializeHdr(const char *filename, int channels, purrSampler *sampler = purrSampler::getDefault(), bool mipmaps = true);
    void cleanup();
    void resize(int width, int height);

    void bind(fr::frPipeline *pipeline, uint32_t set);

    void setPixels(std::vector<uint8_t> pixels);
    void setPixels(uint8_t *pixels, size_t size);

    VkDescriptorImageInfo getDescImageInfo() const {
      return VkDescriptorImageInfo{
        mSampler->mSampler->get(),
        mImage->getView(),
        mColor?VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
      };
    }

    static purrTexture *getBlankTexture();

    static void cleanupAll();

    static void setContext(PurrfectEngineContext *context);
  public:
    fr::frImage *getImage() const { return mImage; }
    fr::frDescriptor *getDescriptor() const { return mDescriptor; }
    void getSize(int *w, int *h) const { *w = mWidth; *h = mHeight; }
  private:
    template <typename T>
    bool initializeImage(std::vector<T> pixels);
  private:
    int mWidth = 0;
    int mHeight = 0;
    VkFormat mFormat = VK_FORMAT_UNDEFINED;
    bool mMipmaps = true;
    bool mColor = true;
    VkSampleCountFlagBits mSampleCount = VK_SAMPLE_COUNT_1_BIT;

    fr::frCommands *mCommands = nullptr;
    fr::frImage *mImage = nullptr;
    purrSampler *mSampler = nullptr;
    fr::frDescriptor *mDescriptor = nullptr;
  };

}

#endif // PURRENGINE_RENDERER_TEXTURE_HPP_