#ifndef   PURRENGINE_RENDERER_HPP_
#define   PURRENGINE_RENDERER_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <glfw/glfw3.h>

namespace PurrfectEngine {

  struct purrWindowInitInfo {
    const char *title;
    int width, height;
  };

  class purrWindow {
  public:
    purrWindow();
    ~purrWindow();

    bool initialize(purrWindowInitInfo initInfo);
    void cleanup();
  public:
    bool shouldClose() const { return glfwWindowShouldClose(mWindow); }
    GLFWwindow *get() const { return mWindow; }
  private:
    GLFWwindow *mWindow = nullptr;
  };

  class purrRendererExt {
  public:
    virtual bool initialize() = 0;
    virtual bool preUpdate()  = 0;
    virtual bool update()     = 0;
    virtual void cleanup()    = 0;

    void beginFrame(VkCommandBuffer cmdBuf, uint32_t imageIndex) { mCmdBuf = cmdBuf; mImageIndex = imageIndex; }
    void endFrame() { mCmdBuf = VK_NULL_HANDLE; mImageIndex = 0; }
  protected:
    VkCommandBuffer mCmdBuf = VK_NULL_HANDLE;
    uint32_t mImageIndex = 0;
  };

  struct purrRendererSwapchainInfo {
    bool VSync = true;
  };

  struct purrRendererInitInfo {
    std::vector<const char *>  extensions;
    std::vector<const char *> *unsupportedExtensions;
    std::vector<const char *>  layers;
    std::vector<const char *> *unsupportedLayers;
    VkPhysicalDeviceFeatures   deviceFeatures;
    std::vector<const char *>  deviceExtensions;
    std::vector<const char *>  deviceLayers;
    purrRendererSwapchainInfo  swapchainInfo;
  };

  class purrRenderTarget;
  class purrRenderer: public purrExtendable<purrRendererExt> {
    friend class purrBuffer;
    friend class purrSampler;
    friend class purrImage;
    friend class purrTexture;
    friend class purrRenderTarget;
    friend class purrPipeline;
    // Extensions
    friend class purrOffscreenRendererExt;
  public:
    purrRenderer(std::vector<purrRendererExt*> extensions);
    ~purrRenderer();

    bool initialize(purrWindow *window, purrRendererInitInfo initInfo);
    bool render();
    void cleanup();
  public:
    static purrRenderer *getInstance() { return sInstance; }
  public:
    void waitIdle() { vkDeviceWaitIdle(mDevice); }
    glm::ivec2 getSwapchainSize() const { return mSwapSize; }
    const char *getError() const { return mError; }
  private:
    virtual bool initialize_() = 0;
    virtual bool resize_() = 0;
    virtual bool render_(VkCommandBuffer cmdBuf) = 0;
    virtual void cleanup_() = 0;

    virtual VkFormat getRenderTargetFormat() = 0;
    virtual VkFormat getHdrFormat() = 0;
    virtual VkFormat getFormat() = 0;
    virtual VkSampleCountFlagBits getSampleCount() = 0;
    virtual purrRenderTarget *getRenderTarget() = 0;
  private:
    bool recreateSwapchain(purrRendererSwapchainInfo swapchainInfo);
    bool createSwapchain(purrRendererSwapchainInfo swapchainInfo);
    bool createSwapObjs();
    void cleanupSwapchain();
  protected:
    VkShaderModule CreateShaderModule(std::vector<char> code);
    bool FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t *result);
    VkFormat FindSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkCommandBuffer BeginSingleTime();
    void EndSingleTime(VkCommandBuffer cmdBuf);
  protected:
    purrWindow      *mWindow    = nullptr;
    const char      *mError     = nullptr;
    VkInstance       mInstance  = VK_NULL_HANDLE;
    VkSurfaceKHR     mSurface   = VK_NULL_HANDLE;
    VkPhysicalDevice mGPU       = VK_NULL_HANDLE;
    VkDevice         mDevice    = VK_NULL_HANDLE;
    VkQueue          mGraphicsQ = VK_NULL_HANDLE;
    uint32_t         mGraphicsF = UINT32_MAX;
    VkQueue          mPresentQ  = VK_NULL_HANDLE;
    uint32_t         mPresentF  = UINT32_MAX;
    VkSwapchainKHR   mSwapchain = VK_NULL_HANDLE;
    VkRenderPass     mRP        = VK_NULL_HANDLE;
    VkCommandPool    mRCommandP = VK_NULL_HANDLE;
    VkCommandBuffer *mRCommands = VK_NULL_HANDLE;
    VkSemaphore     *mISemaphs  = VK_NULL_HANDLE;
    VkSemaphore     *mRSemaphs  = VK_NULL_HANDLE;
    VkFence         *mFFences   = VK_NULL_HANDLE;
    VkPipeline       mPipeline  = VK_NULL_HANDLE;
    VkPipelineLayout mPipelineL = VK_NULL_HANDLE;
    VkCommandPool    mSCommands = VK_NULL_HANDLE;
    VkDescriptorPool mTextDescs = VK_NULL_HANDLE;
    glm::ivec2       mSwapSize  = {};

    VkDescriptorSetLayout mTextureLayout = VK_NULL_HANDLE;

    VkFormat mSwapchainFormat = VK_FORMAT_UNDEFINED;
    VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D mSwapchainExtent;
    std::vector<VkImage> mSwapchainImages{};
    std::vector<VkImageView> mSwapchainImageViews{};
    std::vector<VkFramebuffer> mSwapchainFramebuffers{};

    purrRendererSwapchainInfo mSwapchainInfo{};

    uint32_t mImageCount = 0;
    uint32_t mFramesInFlight = 0;
    uint32_t mFrame = 0;
  private:
    inline static purrRenderer *sInstance = nullptr;
  };

  class purrOffscreenRendererExt: public purrRendererExt {
  public:
    purrOffscreenRendererExt();
    ~purrOffscreenRendererExt();

    virtual bool initialize() override;
    virtual bool preUpdate()  override;
    virtual bool update()     override;
    virtual void cleanup()    override;
  private:
  };

  class purrBuffer {
  public:
    purrBuffer();
    ~purrBuffer();

    VkResult initialize(VkDeviceSize size, VkBufferUsageFlagBits usage, bool local);
    void cleanup();

    void copy(void *data, VkDeviceSize size, VkDeviceSize offset);
  private:
    VkBuffer mBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mMemory = VK_NULL_HANDLE;
  };

  struct purrImageInitInfo {
    VkFormat              format;
    VkExtent3D            extent;
    uint32_t              arrayLayers;
    VkImageType           type;
    VkImageViewType       viewType;
    VkImageAspectFlagBits aspect;
    VkSampleCountFlagBits samples;
  };

  class purrImage {
    friend class purrTexture;
  public:
    purrImage();
    ~purrImage();

    VkResult initialize(purrImageInitInfo initInfo);
    void cleanup();

    VkResult copyFromBuffer(purrBuffer *src);
    VkResult copyToBuffer(purrBuffer *dst);
    VkResult copyFromImage(purrImage *src);

    VkResult transitionLayout(VkImageLayout layout, VkPipelineStageFlagBits stage, VkAccessFlags accessFlag);
  public:
    VkImageView getView() const { return mView; }
    VkImageLayout getLayout() const { return mLayout; }
  private:
    purrImageInitInfo mInitInfo;
  private:
    VkImage         mImage         = VK_NULL_HANDLE;
    VkImageView     mView          = VK_NULL_HANDLE;
    VkDeviceMemory  mMemory        = VK_NULL_HANDLE;

    VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlagBits mStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags mAccessFlag = 0;
  };

  struct purrSamplerInitInfo {
    VkBool32 anisotropyEnable;
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
    VkBorderColor borderColor;
  };

  class purrSampler {
  public:
    purrSampler();
    ~purrSampler();

    VkResult initialize(purrSamplerInitInfo initInfo);
    void cleanup();
  public:
    VkSampler getSampler() const { return mSampler; }
  private:
    VkSampler mSampler = VK_NULL_HANDLE;
  };

  struct purrTextureInitInfo {
    int width, height;
    const char *filename;
    bool hdr;
    VkFormat format;
  };

  class purrPipeline;
  class purrTexture {
  public:
    purrTexture();
    virtual ~purrTexture() = default;
  public:
    VkImageView getView() const { return mImage->getView(); }
  public:
    void bind(VkCommandBuffer cmdBuf, VkPipelineLayout layout, uint32_t set);
    void bind(VkCommandBuffer cmdBuf, purrPipeline *pipeline, uint32_t set);
  protected:
    VkResult createTextureSet();
    void writeImageToSet(VkSampler sampler);
  protected:
    purrImage *mImage = nullptr;
    VkDescriptorSet mSet = VK_NULL_HANDLE;
  };

  struct purrTexture2DInitInfo {
    int                   width;
    int                   height;
    VkFormat              format;
    bool                  color;
    VkSampleCountFlagBits samples;
    purrSampler          *sampler;
  };

  class purrTexture2D: public purrTexture {
  public:
    purrTexture2D();
    virtual ~purrTexture2D() override;

    VkResult initialize(purrTexture2DInitInfo initInfo);
    void cleanup();
  };

  struct purrRenderTargetInitInfo {
    VkExtent3D   extent;
    purrSampler *sampler;
    purrTexture *colorImage;
    purrTexture *depthImage;
    bool         depth;
  };

  class purrRenderTarget {
    friend class purrPipeline;
  public:
    purrRenderTarget();
    ~purrRenderTarget();

    VkResult initialize(purrRenderTargetInitInfo initInfo);
    void cleanup();

    void begin(VkCommandBuffer cmdBuf);
    void end(VkCommandBuffer cmdBuf);
  public:
    purrTexture *getColor() const { return mInitInfo.colorImage; }
    purrTexture *getDepth() const { return mInitInfo.depthImage; }
  private:
    purrRenderTargetInitInfo mInitInfo;
  private:
    VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
    VkRenderPass  mRenderPass  = VK_NULL_HANDLE;
  };

  struct purrPipelineInitInfo {
    std::vector<std::pair<VkShaderStageFlagBits, const char *>> shaders;
    VkSampleCountFlagBits samples;
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkPushConstantRange> pushConstants;
    purrRenderTarget *target;
  };

  class purrPipeline {
  public:
    purrPipeline();
    ~purrPipeline();

    VkResult initialize(purrPipelineInitInfo initInfo);
    void cleanup();

    void bind(VkCommandBuffer cmdBuf);
  public:
    VkPipelineLayout getLayout() const { return mLayout; }
    VkPipeline get() const { return mPipeline; }
  private:
    purrPipelineInitInfo mInitInfo;
  private:
    VkPipelineLayout mLayout   = VK_NULL_HANDLE;
    VkPipeline       mPipeline = VK_NULL_HANDLE;
  };

}

#include "PurrfectEngine/renderer3D.hpp"

#endif // PURRENGINE_RENDERER_HPP_