#include "PurrfectEngine/PurrfectEngine.hpp"

#include <inttypes.h>

namespace PurrfectEngine {

  struct CameraUBO {
    glm::mat4 projection;
    glm::mat4 view;
  };

  static bool sVSync = false;
  static bool sScDirty = false;
  static bool sScPassDirty = false;
  static uint32_t sImageCount = 0;
  static uint32_t sFrame = 0;
  static uint32_t sImageIndex = 0;

  static fr::frDescriptor *sSceneDescriptor = nullptr;

  static fr::frBuffer *sCameraBuffer = nullptr;
  static fr::frDescriptor *sCameraUBO = nullptr;
  
  static fr::frBuffer *sTransformsBuffer = nullptr;
  static uint32_t sTransformsBufCap = 0;
  static bool sTransformsBufDirty = true;
  static fr::frDescriptor *sTransformsDesc = nullptr;

  static purrPipeline *sScenePipeline = nullptr;

  #define IMAGE_NAME_FMT "Swapchain Image %u"
  #define FRAMEBUFFER_NAME_FMT "Swapchain Framebuffer %u"

  std::vector<char> vertexShader_program = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 13, 0, 31, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 9, 0, 0, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 13, 0, 0, 0, 18, 0, 0, 0, 28, 0, 0, 0, 29, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, -62, 1, 0, 0, 4, 0, 10, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 99, 112, 112, 95, 115, 116, 121, 108, 101, 95, 108, 105, 110, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 0, 4, 0, 8, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 105, 110, 99, 108, 117, 100, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 6, 0, 11, 0, 0, 0, 103, 108, 95, 80, 101, 114, 86, 101, 114, 116, 101, 120, 0, 0, 0, 0, 6, 0, 6, 0, 11, 0, 0, 0, 0, 0, 0, 0, 103, 108, 95, 80, 111, 115, 105, 116, 105, 111, 110, 0, 6, 0, 7, 0, 11, 0, 0, 0, 1, 0, 0, 0, 103, 108, 95, 80, 111, 105, 110, 116, 83, 105, 122, 101, 0, 0, 0, 0, 6, 0, 7, 0, 11, 0, 0, 0, 2, 0, 0, 0, 103, 108, 95, 67, 108, 105, 112, 68, 105, 115, 116, 97, 110, 99, 101, 0, 6, 0, 7, 0, 11, 0, 0, 0, 3, 0, 0, 0, 103, 108, 95, 67, 117, 108, 108, 68, 105, 115, 116, 97, 110, 99, 101, 0, 5, 0, 3, 0, 13, 0, 0, 0, 0, 0, 0, 0, 5, 0, 4, 0, 18, 0, 0, 0, 105, 110, 80, 111, 115, 0, 0, 0, 5, 0, 4, 0, 28, 0, 0, 0, 111, 117, 116, 85, 86, 0, 0, 0, 5, 0, 4, 0, 29, 0, 0, 0, 105, 110, 85, 86, 0, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 0, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 1, 0, 0, 0, 11, 0, 0, 0, 1, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 2, 0, 0, 0, 11, 0, 0, 0, 3, 0, 0, 0, 72, 0, 5, 0, 11, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 4, 0, 0, 0, 71, 0, 3, 0, 11, 0, 0, 0, 2, 0, 0, 0, 71, 0, 4, 0, 18, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 28, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 29, 0, 0, 0, 30, 0, 0, 0, 1, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 21, 0, 4, 0, 8, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 1, 0, 0, 0, 28, 0, 4, 0, 10, 0, 0, 0, 6, 0, 0, 0, 9, 0, 0, 0, 30, 0, 6, 0, 11, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 10, 0, 0, 0, 10, 0, 0, 0, 32, 0, 4, 0, 12, 0, 0, 0, 3, 0, 0, 0, 11, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 3, 0, 0, 0, 21, 0, 4, 0, 14, 0, 0, 0, 32, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 14, 0, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 23, 0, 4, 0, 16, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 17, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 18, 0, 0, 0, 1, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 21, 0, 0, 0, 0, 0, -128, 63, 32, 0, 4, 0, 25, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 32, 0, 4, 0, 27, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 59, 0, 4, 0, 27, 0, 0, 0, 28, 0, 0, 0, 3, 0, 0, 0, 59, 0, 4, 0, 17, 0, 0, 0, 29, 0, 0, 0, 1, 0, 0, 0, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, -8, 0, 2, 0, 5, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 19, 0, 0, 0, 18, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 22, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 23, 0, 0, 0, 19, 0, 0, 0, 1, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 24, 0, 0, 0, 22, 0, 0, 0, 23, 0, 0, 0, 20, 0, 0, 0, 21, 0, 0, 0, 65, 0, 5, 0, 25, 0, 0, 0, 26, 0, 0, 0, 13, 0, 0, 0, 15, 0, 0, 0, 62, 0, 3, 0, 26, 0, 0, 0, 24, 0, 0, 0, 61, 0, 4, 0, 16, 0, 0, 0, 30, 0, 0, 0, 29, 0, 0, 0, 62, 0, 3, 0, 28, 0, 0, 0, 30, 0, 0, 0, -3, 0, 1, 0, 56, 0, 1, 0};
  std::vector<char> fragmentShader_program = {3, 2, 35, 7, 0, 0, 1, 0, 11, 0, 13, 0, 27, 0, 0, 0, 0, 0, 0, 0, 17, 0, 2, 0, 1, 0, 0, 0, 11, 0, 6, 0, 1, 0, 0, 0, 71, 76, 83, 76, 46, 115, 116, 100, 46, 52, 53, 48, 0, 0, 0, 0, 14, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 15, 0, 7, 0, 4, 0, 0, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 9, 0, 0, 0, 17, 0, 0, 0, 16, 0, 3, 0, 4, 0, 0, 0, 7, 0, 0, 0, 3, 0, 3, 0, 2, 0, 0, 0, -62, 1, 0, 0, 4, 0, 10, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 99, 112, 112, 95, 115, 116, 121, 108, 101, 95, 108, 105, 110, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 0, 4, 0, 8, 0, 71, 76, 95, 71, 79, 79, 71, 76, 69, 95, 105, 110, 99, 108, 117, 100, 101, 95, 100, 105, 114, 101, 99, 116, 105, 118, 101, 0, 5, 0, 4, 0, 4, 0, 0, 0, 109, 97, 105, 110, 0, 0, 0, 0, 5, 0, 5, 0, 9, 0, 0, 0, 111, 117, 116, 67, 111, 108, 111, 114, 0, 0, 0, 0, 5, 0, 5, 0, 13, 0, 0, 0, 117, 83, 97, 109, 112, 108, 101, 114, 0, 0, 0, 0, 5, 0, 4, 0, 17, 0, 0, 0, 105, 110, 85, 86, 0, 0, 0, 0, 71, 0, 4, 0, 9, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 34, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 13, 0, 0, 0, 33, 0, 0, 0, 0, 0, 0, 0, 71, 0, 4, 0, 17, 0, 0, 0, 30, 0, 0, 0, 0, 0, 0, 0, 19, 0, 2, 0, 2, 0, 0, 0, 33, 0, 3, 0, 3, 0, 0, 0, 2, 0, 0, 0, 22, 0, 3, 0, 6, 0, 0, 0, 32, 0, 0, 0, 23, 0, 4, 0, 7, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 32, 0, 4, 0, 8, 0, 0, 0, 3, 0, 0, 0, 7, 0, 0, 0, 59, 0, 4, 0, 8, 0, 0, 0, 9, 0, 0, 0, 3, 0, 0, 0, 25, 0, 9, 0, 10, 0, 0, 0, 6, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 27, 0, 3, 0, 11, 0, 0, 0, 10, 0, 0, 0, 32, 0, 4, 0, 12, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 59, 0, 4, 0, 12, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 23, 0, 4, 0, 15, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 32, 0, 4, 0, 16, 0, 0, 0, 1, 0, 0, 0, 15, 0, 0, 0, 59, 0, 4, 0, 16, 0, 0, 0, 17, 0, 0, 0, 1, 0, 0, 0, 23, 0, 4, 0, 20, 0, 0, 0, 6, 0, 0, 0, 3, 0, 0, 0, 43, 0, 4, 0, 6, 0, 0, 0, 22, 0, 0, 0, 0, 0, -128, 63, 54, 0, 5, 0, 2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, -8, 0, 2, 0, 5, 0, 0, 0, 61, 0, 4, 0, 11, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 61, 0, 4, 0, 15, 0, 0, 0, 18, 0, 0, 0, 17, 0, 0, 0, 87, 0, 5, 0, 7, 0, 0, 0, 19, 0, 0, 0, 14, 0, 0, 0, 18, 0, 0, 0, 79, 0, 8, 0, 20, 0, 0, 0, 21, 0, 0, 0, 19, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 23, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 24, 0, 0, 0, 21, 0, 0, 0, 1, 0, 0, 0, 81, 0, 5, 0, 6, 0, 0, 0, 25, 0, 0, 0, 21, 0, 0, 0, 2, 0, 0, 0, 80, 0, 7, 0, 7, 0, 0, 0, 26, 0, 0, 0, 23, 0, 0, 0, 24, 0, 0, 0, 25, 0, 0, 0, 22, 0, 0, 0, 62, 0, 3, 0, 9, 0, 0, 0, 26, 0, 0, 0, -3, 0, 1, 0, 56, 0, 1, 0};

  VkCommandBuffer *sCmdBufs{};
  std::vector<fr::frSynchronization*> sSynchronizations{};

  static PurrfectEngineContext *sContext;

  void createSwapchain() {
    sContext->frSwapchain = new fr::frSwapchain();
    if (sVSync) {
      sContext->frSwapchain->setDesiredPresentModes({ VK_PRESENT_MODE_FIFO_KHR });
    } else {
      sContext->frSwapchain->setDesiredPresentModes({ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR });
    }
    sContext->frSwapchain->initialize(sContext->frRenderer, sContext->frWindow);
    sImageCount = sContext->frSwapchain->imageCount();
  }

  void createSwapchainObjects() {
    sContext->frScImages.resize(sImageCount);
    sContext->frFbs.resize(sImageCount);

    int w, h;
    sContext->frSwapchain->getSize(&w, &h);
    for (uint32_t i = 0; i < sImageCount; ++i) {
      fr::frImage *image = new fr::frImage();
      image->initialize(sContext->frRenderer, sContext->frSwapchain->getImage(i), fr::frImage::frImageInfo{
        w, h, sContext->frSwapchain->format(),
        (VkImageUsageFlagBits)(0),
        false, 0
      });
      char buf[32] = {0};
      sprintf(buf, IMAGE_NAME_FMT, i);
      image->setName(sContext->frRenderer, buf);
      sContext->frScImages[i] = image;

      fr::frFramebuffer *fb = new fr::frFramebuffer();
      fb->initialize(sContext->frRenderer, w, h, sContext->frRenderPass, { sContext->frScImages[i] });
      sprintf(buf, FRAMEBUFFER_NAME_FMT, i);
      fb->setName(sContext->frRenderer, buf);
      sContext->frFbs[i] = fb;
    }
  }

  void cleanupSwapchain() {
    for (uint32_t i = 0; i < sImageCount; ++i) {
      delete sContext->frScImages[i];
      delete sContext->frFbs[i];
    }
    delete sContext->frSwapchain;
  }

  void recreateSwapchain() {
    auto p = sContext->frWindow->getSize();
    int w = p.first, h = p.second;
    while (w == 0 || h == 0) {
      p = sContext->frWindow->getSize();
      w = p.first; h = p.second;
      glfwWaitEvents();
    }

    sContext->frRenderer->waitIdle();

    cleanupSwapchain();
    createSwapchain();
    createSwapchainObjects();
  }

  void createScenePass() {
    VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkAttachmentReference depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    sContext->frSceneRenderPass = new fr::frRenderPass();
    sContext->frSceneRenderPass->addAttachment(VkAttachmentDescription{
      0, sContext->frSceneFormat, (VkSampleCountFlagBits)sContext->frMsaa,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    });

    sContext->frSceneRenderPass->addAttachment(VkAttachmentDescription{
      0, sContext->frDepthFormat, (VkSampleCountFlagBits)sContext->frMsaa,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    });

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    sContext->frSceneRenderPass->addSubpass(subpass);

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    sContext->frSceneRenderPass->addDependency(dependency);

    sContext->frSceneRenderPass->initialize(sContext->frRenderer);
    sContext->frSceneRenderPass->setName(sContext->frRenderer, "Scene RenderPass");
  }

  void cleanupScenePass() {
    delete sContext->frSceneRenderPass;
  }

  void recreateScenePass() {
    cleanupScenePass();
    createScenePass();
  }

  void createRenderPass() {
    sContext->frRenderPass = new fr::frRenderPass();
    sContext->frRenderPass->addAttachment(VkAttachmentDescription{
      0, sContext->frSwapchain->format(), (VkSampleCountFlagBits)sContext->frMsaa,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    });

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    sContext->frRenderPass->addSubpass(subpass);
    sContext->frRenderPass->addDependency(dependency);
    sContext->frRenderPass->initialize(sContext->frRenderer);
    sContext->frRenderPass->setName(sContext->frRenderer, "Swapchain RP");
  }

  void createSwapchainPipeline() {
    sContext->frPipeline = new fr::frPipeline();

    auto vertShader = new fr::frShader();
    auto fragShader = new fr::frShader();
    { // Vertex shader
      vertShader->initialize(sContext->frRenderer, vertexShader_program, VK_SHADER_STAGE_VERTEX_BIT);
      sContext->frPipeline->addShader(vertShader);
    }
    { // Fragment shader
      fragShader->initialize(sContext->frRenderer, fragmentShader_program, VK_SHADER_STAGE_FRAGMENT_BIT);
      sContext->frPipeline->addShader(fragShader);
    }

    sContext->frPipeline->setVertexInputState<Vertex2D>();

    sContext->frPipeline->setMultisampleInfo(VkPipelineMultisampleStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      (VkSampleCountFlagBits)sContext->frMsaa, VK_FALSE, 0.0f, VK_NULL_HANDLE,
      VK_FALSE, VK_FALSE
    });

    sContext->frPipeline->setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
    });

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    sContext->frPipeline->setColorBlendState(VkPipelineColorBlendStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_FALSE,
      VK_LOGIC_OP_COPY,
      1,
      &colorBlendAttachment,
      {0.0f, 0.0f, 0.0f, 0.0f}
    });

    sContext->frPipeline->setRasterizationState(VkPipelineRasterizationStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE,
      VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    });

    std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    sContext->frPipeline->setDynamicState(VkPipelineDynamicStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data()
    });

    sContext->frPipeline->setViewportState(VkPipelineViewportStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      1, nullptr, 1, nullptr
    });

    sContext->frPipeline->addDescriptor(sContext->frTextureLayout);

    sContext->frPipeline->initialize(sContext->frRenderer, sContext->frRenderPass);

    delete vertShader;
    delete fragShader;
  }

  void cleanupRenderPass() {
    delete sContext->frRenderPass;
    delete sContext->frPipeline;
  }

  void recreateRenderPass() {
    cleanupRenderPass();
    createRenderPass();
    createSwapchainPipeline();
  }

  void renderer::setContext(PurrfectEngineContext *context) {
    sContext = context;
    purrTexture::setContext(context);
    purrMesh::setContext(context);
    purrPipeline::setContext(context);
  }

  void renderer::setScene(purrScene *scene) {
    sContext->activeScene = scene;
  }

  void renderer::setVSync(bool enabled) {
    if (sVSync == enabled) return;
    sVSync = enabled;
    sScDirty = true;
  }

  void renderer::setMSAA(MSAA msaa) {
    if (sContext->frMsaa == msaa) return;
    sContext->frMsaa = msaa;
    sScPassDirty = true;
  }

  void renderer::initialize(std::string title, int width, int height) {
    sContext->frWindow = new fr::frWindow(title, width, height);

    sContext->frRenderer = new fr::frRenderer();

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

    sContext->frWindow->addExtensions(sContext->frRenderer);
    #if 1
    sContext->frRenderer->addLayer("VK_LAYER_KHRONOS_validation");
    sContext->frRenderer->addExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    sContext->frRenderer->addExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    #endif
    sContext->frRenderer->initialize(sContext->frWindow, &physicalDeviceFeatures);

    createSwapchain();

    createRenderPass();

    sContext->frTextureLayout = new fr::frDescriptorLayout();
    sContext->frTextureLayout->addBinding(VkDescriptorSetLayoutBinding{
      0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
      VK_SHADER_STAGE_FRAGMENT_BIT,
      VK_NULL_HANDLE
    });
    sContext->frTextureLayout->initialize(sContext->frRenderer);

    sContext->frUboLayout = new fr::frDescriptorLayout();
    sContext->frUboLayout->addBinding(VkDescriptorSetLayoutBinding{
      0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_NULL_HANDLE
    });
    sContext->frUboLayout->initialize(sContext->frRenderer);

    sContext->frStorageBufLayout = new fr::frDescriptorLayout();
    sContext->frStorageBufLayout->addBinding(VkDescriptorSetLayoutBinding{
      0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
      VK_SHADER_STAGE_VERTEX_BIT,
      VK_NULL_HANDLE
    });
    sContext->frStorageBufLayout->initialize(sContext->frRenderer);

    createSwapchainPipeline();

    createSwapchainObjects();

    sContext->frCommands = new fr::frCommands();
    sContext->frCommands->initialize(sContext->frRenderer);

    sCmdBufs = sContext->frCommands->allocateBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, sImageCount);

    sContext->frDescriptors = new fr::frDescriptors();
    sContext->frDescriptors->initialize(sContext->frRenderer, {
      { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
      { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
    });

    sContext->frTextureDescriptors = new fr::frDescriptors();
    sContext->frTextureDescriptors->initialize(sContext->frRenderer, {
      { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048 },
    });

    for (size_t i = 0; i < sImageCount; ++i) {
      fr::frSynchronization *sync = new fr::frSynchronization();
      sync->initialize(sContext->frRenderer);
      sSynchronizations.push_back(sync);
    }

    sContext->frDepthFormat = sContext->frRenderer->FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    sContext->frSceneFormat = sContext->frRenderer->FindSupportedFormat({VK_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
   
    createScenePass();
  }

  void renderer::getSwapchainSize(int *width, int *height) {
    sContext->frSwapchain->getSize(width, height);
  }

  void renderer::setScenePipeline(purrPipeline *scenePipeline) {
    sScenePipeline = scenePipeline;

    sSceneDescriptor = sScenePipeline->getColor()->getDescriptor();
  }

  void renderer::updateCamera() {
    if (!sCameraBuffer) {
      sCameraBuffer = new fr::frBuffer();
      sCameraBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        sizeof(CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });

      VkDescriptorBufferInfo bufferInfo = {
        sCameraBuffer->get(), 0, sizeof(CameraUBO)
      };
      sCameraUBO = sContext->frDescriptors->allocate(1, sContext->frUboLayout)[0];
      sCameraUBO->update(fr::frDescriptor::frDescriptorWriteInfo{
        0, 0, 1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_NULL_HANDLE, &bufferInfo, VK_NULL_HANDLE
      });
    }

    if (!sContext->activeScene) return;
    purrObject *cameraObj = sContext->activeScene->getCamera();
    if (!cameraObj) return;
    purrCameraComp *cameraComp = (purrCameraComp*)cameraObj->getComponent("cameraComponent");
    if (!cameraComp) return;
    purrCamera *camera = cameraComp->getCamera();

    CameraUBO cameraUbo = {};
    cameraUbo.projection = camera->getProjection();
    cameraUbo.view = camera->getView();
    sCameraBuffer->copyData(0, sizeof(cameraUbo), &cameraUbo);
  }

  void renderer::updateTransforms() {
    VkDeviceSize bufSize = sizeof(glm::mat4) * (sTransformsBufCap>0?sTransformsBufCap:(sTransformsBufCap=256));
    bool cleanup = true;
    if (!sTransformsBuffer) {
      sTransformsBuffer = new fr::frBuffer();
      sTransformsBufDirty = true;
      cleanup = false;
    }

    if (sTransformsBufDirty) {
      if (cleanup) sTransformsBuffer->cleanup();
      sTransformsBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      sTransformsBufDirty = false;
    }

    if (!sTransformsDesc) {
      VkDescriptorBufferInfo bufferInfo = {
        sTransformsBuffer->get(), 0, bufSize
      };
      sTransformsDesc = sContext->frDescriptors->allocate(1, sContext->frStorageBufLayout)[0];
      sTransformsDesc->update(fr::frDescriptor::frDescriptorWriteInfo{
        0, 0, 1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_NULL_HANDLE, &bufferInfo, VK_NULL_HANDLE
      });
    }

    if (!sContext->activeScene) return;
    std::vector<purrObject*> objects = sContext->activeScene->getObjects();

    if (static_cast<uint32_t>(objects.size()) >= sTransformsBufCap) {
      while (static_cast<uint32_t>(objects.size()) >= sTransformsBufCap) sTransformsBufCap*=2;
      sTransformsBufDirty = true;
      updateTransforms();
      return;
    }

    std::vector<glm::mat4> transforms{};
    transforms.reserve(objects.size());
    for (purrObject *object: objects) transforms.push_back(object->getTransform()->getTransform());
    uint32_t size = static_cast<uint32_t>(transforms.size());
    sTransformsBuffer->copyData(0, sizeof(glm::mat4)*size, transforms.data());
  }

  bool renderer::shouldClose() {
    return sContext->frWindow->shouldClose();
  }

  bool renderer::renderBegin() {
    sSynchronizations[sFrame]->wait();

    try {
      sImageIndex = sContext->frRenderer->acquireNextImage(sContext->frSwapchain, sSynchronizations[sFrame]);
    } catch (fr::frSwapchainResizeException &ex) {
      recreateSwapchain();
      return false;
    }

    sSynchronizations[sFrame]->reset();

    vkResetCommandBuffer(sCmdBufs[sFrame], 0);
    (*sContext).frActiveCmdBuf = sCmdBufs[sFrame];
    fr::frCommands::begin(sCmdBufs[sFrame]);

    return true;
  }

  void renderer::bindCamera(fr::frPipeline *pipeline) {
    pipeline->bindDescriptor(sCmdBufs[sFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 0, sCameraUBO);
  }

  void renderer::bindTransforms(fr::frPipeline *pipeline) {
    pipeline->bindDescriptor(sCmdBufs[sFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 1, sTransformsDesc);
  }

  void renderer::renderScene(purrPipeline *pipeline) {
    purrScene *scene = sContext->activeScene;
    if (!scene) return;
    std::vector<purrObject*> objects = scene->getObjects();
    uint32_t idx = 0;
    for (purrObject *obj: objects) {
      purrComponent *meshComp = nullptr;
      if ((meshComp = obj->getComponent("meshComponent"))) {
        pipeline->get()->pushConstant(sCmdBufs[sFrame],
                                      VK_SHADER_STAGE_VERTEX_BIT,
                                      (uint32_t)0, 
                                      static_cast<uint32_t>(sizeof(uint32_t)), 
                                      (const void*)&idx);
        ((purrMeshComp*)meshComp)->getMesh()->render(sCmdBufs[sFrame]);
      }
      ++idx;
    }
  }

  void renderer::render() {
    std::vector<VkClearValue> clearValues = {};
    clearValues.push_back({{{1.0f, 1.0f, 1.0f, 1.0f}}});

    sContext->frRenderPass->begin(sCmdBufs[sFrame], sContext->frSwapchain->extent(), sContext->frFbs[sImageIndex], clearValues);

    sContext->frPipeline->bind(sCmdBufs[sFrame], VK_PIPELINE_BIND_POINT_GRAPHICS);

    auto scExtent = sContext->frSwapchain->extent();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(scExtent.width);
    viewport.height = static_cast<float>(scExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(sCmdBufs[sFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = scExtent;
    vkCmdSetScissor(sCmdBufs[sFrame], 0, 1, &scissor);

    if (sSceneDescriptor) sContext->frPipeline->bindDescriptor(sCmdBufs[sFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 0, sSceneDescriptor);
    purrMesh2D *squareMesh = purrMesh2D::getSquareMesh();
    squareMesh->render(sCmdBufs[sFrame]);

    sContext->frRenderPass->end(sCmdBufs[sFrame]);
  }

  bool renderer::present() {
    fr::frCommands::end(sCmdBufs[sFrame]);
    (*sContext).frActiveCmdBuf = VK_NULL_HANDLE;
    fr::frCommands::submit(sContext->frRenderer, sCmdBufs[sFrame], sSynchronizations[sFrame]);
    try {
      sContext->frRenderer->present(sContext->frSwapchain, sSynchronizations[sFrame], &sImageIndex);
    } catch (fr::frSwapchainResizeException &ex) {
      sScDirty = true;
    }

    bool res = !sScDirty;
    if (sScDirty) {
      recreateSwapchain();
      sScDirty = false;
    }
    
    if (sScPassDirty) { // Recreate scene pass (MSAA changed)
      recreateRenderPass();
      recreateScenePass();
      sScPassDirty = false;
    }

    sFrame = (sFrame+1) % sContext->frSwapchain->imageCount();
    return res;
  }

  void renderer::waitIdle() {
    sContext->frRenderer->waitIdle();
  }

  void renderer::cleanup() {
    purrSampler::cleanupAll();
    purrMesh::cleanupAll();
    purrMesh2D::cleanupAll();

    delete sContext->frWindow;
    cleanupSwapchain();
    if (sCameraBuffer) delete sCameraBuffer;
    if (sTransformsBuffer) delete sTransformsBuffer;
    cleanupScenePass();
    cleanupRenderPass();
    delete sContext->frCommands;
    delete sContext->frDescriptors;
    delete sContext->frTextureDescriptors;
    delete sContext->frTextureLayout;
    delete sContext->frUboLayout;
    delete sContext->frStorageBufLayout;
    for (auto sync: sSynchronizations) delete sync;
    delete sContext->frRenderer;
  }

  size_t Utils::formatToChannels(VkFormat format) {
    switch (format) {
    case VK_FORMAT_R8_UNORM:             return 1;
    case VK_FORMAT_R8_SNORM:             return 2;
    case VK_FORMAT_R8_USCALED:           return 1;
    case VK_FORMAT_R8_SSCALED:           return 1;
    case VK_FORMAT_R8_UINT:              return 1;
    case VK_FORMAT_R8_SINT:              return 1;
    case VK_FORMAT_R8_SRGB:              return 1;
    case VK_FORMAT_R16_UNORM:            return 1;
    case VK_FORMAT_R16_SNORM:            return 1;
    case VK_FORMAT_R16_USCALED:          return 1;
    case VK_FORMAT_R16_SSCALED:          return 1;
    case VK_FORMAT_R16_UINT:             return 1;
    case VK_FORMAT_R16_SINT:             return 1;
    case VK_FORMAT_R16_SFLOAT:           return 1;
    case VK_FORMAT_R32_UINT:             return 1;
    case VK_FORMAT_R32_SINT:             return 1;
    case VK_FORMAT_R32_SFLOAT:           return 1;
    case VK_FORMAT_R64_UINT:             return 1;
    case VK_FORMAT_R64_SINT:             return 1;
    case VK_FORMAT_R64_SFLOAT:           return 1;
    case VK_FORMAT_R8G8_UNORM:           return 2;
    case VK_FORMAT_R8G8_SNORM:           return 2;
    case VK_FORMAT_R8G8_USCALED:         return 2;
    case VK_FORMAT_R8G8_SSCALED:         return 2;
    case VK_FORMAT_R8G8_UINT:            return 2;
    case VK_FORMAT_R8G8_SINT:            return 2;
    case VK_FORMAT_R8G8_SRGB:            return 2;
    case VK_FORMAT_R16G16_UNORM:         return 2;
    case VK_FORMAT_R16G16_SNORM:         return 2;
    case VK_FORMAT_R16G16_USCALED:       return 2;
    case VK_FORMAT_R16G16_SSCALED:       return 2;
    case VK_FORMAT_R16G16_UINT:          return 2;
    case VK_FORMAT_R16G16_SINT:          return 2;
    case VK_FORMAT_R16G16_SFLOAT:        return 2;
    case VK_FORMAT_R32G32_UINT:          return 2;
    case VK_FORMAT_R32G32_SINT:          return 2;
    case VK_FORMAT_R32G32_SFLOAT:        return 2;
    case VK_FORMAT_R64G64_UINT:          return 2;
    case VK_FORMAT_R64G64_SINT:          return 2;
    case VK_FORMAT_R64G64_SFLOAT:        return 2;
    case VK_FORMAT_R8G8B8_UNORM:         return 3;
    case VK_FORMAT_R8G8B8_SNORM:         return 3;
    case VK_FORMAT_R8G8B8_USCALED:       return 3;
    case VK_FORMAT_R8G8B8_SSCALED:       return 3;
    case VK_FORMAT_R8G8B8_UINT:          return 3;
    case VK_FORMAT_R8G8B8_SINT:          return 3;
    case VK_FORMAT_R8G8B8_SRGB:          return 3;
    case VK_FORMAT_B8G8R8_UNORM:         return 3;
    case VK_FORMAT_B8G8R8_SNORM:         return 3;
    case VK_FORMAT_B8G8R8_USCALED:       return 3;
    case VK_FORMAT_B8G8R8_SSCALED:       return 3;
    case VK_FORMAT_B8G8R8_UINT:          return 3;
    case VK_FORMAT_B8G8R8_SINT:          return 3;
    case VK_FORMAT_B8G8R8_SRGB:          return 3;
    case VK_FORMAT_R16G16B16_UNORM:      return 3;
    case VK_FORMAT_R16G16B16_SNORM:      return 3;
    case VK_FORMAT_R16G16B16_USCALED:    return 3;
    case VK_FORMAT_R16G16B16_SSCALED:    return 3;
    case VK_FORMAT_R16G16B16_UINT:       return 3;
    case VK_FORMAT_R16G16B16_SINT:       return 3;
    case VK_FORMAT_R16G16B16_SFLOAT:     return 3;
    case VK_FORMAT_R32G32B32_UINT:       return 3;
    case VK_FORMAT_R32G32B32_SINT:       return 3;
    case VK_FORMAT_R32G32B32_SFLOAT:     return 3;
    case VK_FORMAT_R64G64B64_UINT:       return 3;
    case VK_FORMAT_R64G64B64_SINT:       return 3;
    case VK_FORMAT_R64G64B64_SFLOAT:     return 3;
    case VK_FORMAT_R8G8B8A8_UNORM:       return 4;
    case VK_FORMAT_R8G8B8A8_SNORM:       return 4;
    case VK_FORMAT_R8G8B8A8_USCALED:     return 4;
    case VK_FORMAT_R8G8B8A8_SSCALED:     return 4;
    case VK_FORMAT_R8G8B8A8_UINT:        return 4;
    case VK_FORMAT_R8G8B8A8_SINT:        return 4;
    case VK_FORMAT_R8G8B8A8_SRGB:        return 4;
    case VK_FORMAT_B8G8R8A8_UNORM:       return 4;
    case VK_FORMAT_B8G8R8A8_SNORM:       return 4;
    case VK_FORMAT_B8G8R8A8_USCALED:     return 4;
    case VK_FORMAT_B8G8R8A8_SSCALED:     return 4;
    case VK_FORMAT_B8G8R8A8_UINT:        return 4;
    case VK_FORMAT_B8G8R8A8_SINT:        return 4;
    case VK_FORMAT_B8G8R8A8_SRGB:        return 4;
    case VK_FORMAT_R16G16B16A16_UNORM:   return 4;
    case VK_FORMAT_R16G16B16A16_SNORM:   return 4;
    case VK_FORMAT_R16G16B16A16_USCALED: return 4;
    case VK_FORMAT_R16G16B16A16_SSCALED: return 4;
    case VK_FORMAT_R16G16B16A16_UINT:    return 4;
    case VK_FORMAT_R16G16B16A16_SINT:    return 4;
    case VK_FORMAT_R16G16B16A16_SFLOAT:  return 4;
    case VK_FORMAT_R32G32B32A32_UINT:    return 4;
    case VK_FORMAT_R32G32B32A32_SINT:    return 4;
    case VK_FORMAT_R32G32B32A32_SFLOAT:  return 4;
    case VK_FORMAT_R64G64B64A64_UINT:    return 4;
    case VK_FORMAT_R64G64B64A64_SINT:    return 4;
    case VK_FORMAT_R64G64B64A64_SFLOAT:  return 4;
    default: break;
    }
    return 0;
  }

}