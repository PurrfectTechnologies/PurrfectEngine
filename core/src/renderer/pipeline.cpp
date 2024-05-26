#include "PurrfectEngine/PurrfectEngine.hpp"

#include <assert.h>

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext;

  purrPipeline::purrPipeline(purrPipelineCreateInfo createInfo):
    mRenderer(sContext->frRenderer), mRenderPass(new fr::frRenderPass()), mPipeline(new fr::frPipeline()), mCreateInfo(createInfo)
  {
    mColorTexture = createInfo.colorTarget;
    mDepthTexture = createInfo.depthTarget;
    assert(mColorTexture && "colorTarget MUST always be a valid purrTexture object!");

    if (!mDepthTexture) {
      mDepthTexture = new purrTexture(createInfo.width, createInfo.height, sContext->frDepthFormat);
      mDepthTexture->initialize(nullptr, false, false);
    }

    VkAttachmentReference colorRef = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthRef = {
      1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    mRenderPass->addAttachment(VkAttachmentDescription{
      0, mColorTexture->mFormat, mColorTexture->mSampleCount,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    });

    mRenderPass->addAttachment(VkAttachmentDescription{
      0, mDepthTexture->mFormat, mDepthTexture->mSampleCount,
      VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    });

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;
    mRenderPass->addSubpass(subpass);

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    mRenderPass->addDependency(dependency);

    mRenderPass->initialize(sContext->frRenderer);

    mFramebuffer = new fr::frFramebuffer();
    mFramebuffer->initialize(sContext->frRenderer, createInfo.width, createInfo.height, mRenderPass, { mColorTexture->getImage(), mDepthTexture->getImage() });

    mPipeline = new fr::frPipeline();

    for (auto shdr: createInfo.shaders) {
      fr::frShader *shader = new fr::frShader();
      shader->initialize(sContext->frRenderer, shdr.second, shdr.first);
      mPipeline->addShader(shader);
      mShaders.push_back(shader);
    }

    mPipeline->setVertexInputState<Vertex3D>();

    mPipeline->setMultisampleInfo(VkPipelineMultisampleStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      (VkSampleCountFlagBits)sContext->settings.msaa, VK_FALSE, 0.0f, VK_NULL_HANDLE,
      VK_FALSE, VK_FALSE
    });

    mPipeline->setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
    });

    static VkPipelineColorBlendAttachmentState *sColorBlendAttachment = new VkPipelineColorBlendAttachmentState();
    sColorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    sColorBlendAttachment->blendEnable = VK_FALSE;

    mPipeline->setColorBlendState(VkPipelineColorBlendStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_FALSE,
      VK_LOGIC_OP_COPY,
      1,
      sColorBlendAttachment,
      {0.0f, 0.0f, 0.0f, 0.0f}
    });

    mPipeline->setRasterizationState(VkPipelineRasterizationStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
      VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
    });

    mPipeline->setDynamicState(VkPipelineDynamicStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      0, VK_NULL_HANDLE
    });

    VkViewport *viewport = new VkViewport({ 0,0,static_cast<float>(createInfo.width),static_cast<float>(createInfo.height),0.0f,1.0f });
    VkRect2D *scissor = new VkRect2D({ {}, { static_cast<uint32_t>(createInfo.width), static_cast<uint32_t>(createInfo.height) } });
    mPipeline->setViewportState(VkPipelineViewportStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      1, viewport, 1, scissor
    });

    mPipeline->setDepthStencilState(VkPipelineDepthStencilStateCreateInfo{
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
      VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS,
      VK_FALSE, VK_FALSE,
      {}, {},
      1.0f, 1.0f
    });

    mPipeline->addDescriptor(sContext->frUboLayout);
  }

  purrPipeline::~purrPipeline() {
    cleanup();
  }

  void purrPipeline::initialize() {
    mPipeline->initialize(mRenderer, mRenderPass);

    for (auto shader: mShaders) delete shader;
  }

  void purrPipeline::cleanup() {
    if (mDepthTexture != mCreateInfo.depthTarget) delete mDepthTexture;

    delete mRenderPass;
    delete mFramebuffer;
    delete mPipeline;
  }

  void purrPipeline::begin(VkClearValue clearColor) {
    std::vector<VkClearValue> clearValues = {clearColor, {{1.0f, 0}}};
    mRenderPass->begin(sContext->frActiveCmdBuf, VkExtent2D{static_cast<uint32_t>(mCreateInfo.width), static_cast<uint32_t>(mCreateInfo.height)}, mFramebuffer, clearValues);
    mPipeline->bind(sContext->frActiveCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);
    renderer::bindCamera(mPipeline);
  }

  void purrPipeline::end() {
    mRenderPass->end(sContext->frActiveCmdBuf);
  }

  void purrPipeline::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

}