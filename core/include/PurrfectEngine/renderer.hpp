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
  class purrRenderer {
    friend class purrBuffer;
    friend class purrSampler;
    friend class purrImage;
    friend class purrRenderTarget;
  public:
    purrRenderer();
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
    virtual bool render_() = 0;
    virtual void cleanup_() = 0;

    virtual VkFormat getRenderTargetFormat() = 0;
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
    glm::ivec2       mSwapSize  = {};

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

  class purrRenderer3D: public purrRenderer {
  public:
    purrRenderer3D();
    ~purrRenderer3D();
  private:
    virtual bool initialize_() override;
    virtual bool render_()     override;
    virtual void cleanup_()    override;

    virtual VkFormat getRenderTargetFormat() override;
    virtual VkSampleCountFlagBits getSampleCount() override;
    virtual purrRenderTarget *getRenderTarget() override;
  private:
    purrRenderTarget *mRenderTarget = nullptr;
  };

  class purrBuffer {
  public:

  private:
  };

  struct purrImageInitInfo {
    VkFormat              format;
    VkExtent3D            extent;
    uint32_t              arrayLayers;
    VkImageType           type;
    VkImageViewType       viewType;
    VkImageAspectFlagBits aspect;
    VkSampleCountFlagBits samples;
    bool                  sampled;
  };

  class purrImage {
  public:
    purrImage();
    ~purrImage();

    VkResult initialize(purrImageInitInfo initInfo);
    void cleanup();

    VkResult createSet(VkSampler sampler); // TODO: Add `purrSampler` class

    VkResult copyFromBuffer(purrBuffer *src);
    VkResult copyToBuffer(purrBuffer *dst);
    VkResult copyFromImage(purrImage *src);

    VkResult transitionLayout(VkImageLayout layout, VkPipelineStageFlagBits stage, VkAccessFlags accessFlag);
  public:
    VkImageView getView() const { return mView; }
  private:
    purrImageInitInfo mInitInfo;
  private:
    VkImage         mImage         = VK_NULL_HANDLE;
    VkImageView     mView          = VK_NULL_HANDLE;
    VkDeviceMemory  mMemory        = VK_NULL_HANDLE;
    VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;

    VkImageLayout mLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkPipelineStageFlagBits mStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags mAccessFlag = 0;
  };

  struct purrRenderTargetInitInfo {
    VkExtent3D extent;
    purrImage *colorImage; // Can be nullptr to make `purrRenderTarget` create new one.
    purrImage *depthImage; // Can be nullptr to make `purrRenderTarget` create new one.
    bool       depth;
  };

  class purrRenderTarget {
  public:
    purrRenderTarget();
    ~purrRenderTarget();

    VkResult initialize(purrRenderTargetInitInfo initInfo);
    void cleanup();

    void begin(VkCommandBuffer cmdBuf);
    void end(VkCommandBuffer cmdBuf);
  private:
    purrRenderTargetInitInfo mInitInfo;
  private:
    VkFramebuffer mFramebuffer = VK_NULL_HANDLE;
    VkRenderPass  mRenderPass  = VK_NULL_HANDLE;
  };

}

// #include "PurrfectEngine/renderer/texture.hpp"
// #include "PurrfectEngine/renderer/renderTarget.hpp"
// #include "PurrfectEngine/renderer/cubemap.hpp"
// #include "PurrfectEngine/renderer/skybox.hpp"
// #include "PurrfectEngine/renderer/mesh.hpp"
// #include "PurrfectEngine/renderer/pipeline.hpp"

#endif // PURRENGINE_RENDERER_HPP_