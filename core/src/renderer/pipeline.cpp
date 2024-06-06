#include "PurrfectEngine/PurrfectEngine.hpp"

#include <assert.h>

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  purrPipeline::purrPipeline()
  {}

  purrPipeline::~purrPipeline() {
    cleanup();
  }

  void purrPipeline::initialize(purrPipelineCreateInfo createInfo) {
    {
      mPipeline = new fr::frPipeline();

      for (auto shdr: createInfo.shaders) {
        fr::frShader *shader = new fr::frShader();
        shader->initialize(sContext->frRenderer, shdr.second, shdr.first);
        mPipeline->addShader(shader);
        mShaders.push_back(shader);
      }

      mPipeline->setVertexInputState<Vertex3D>();

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
      mPipeline->addDescriptor(sContext->frStorageBufLayout);
      mPipeline->addDescriptor(sContext->frStorageBufLayout);
      mPipeline->addDescriptor(sContext->frSkyboxLayout);
      mPipeline->addDescriptor(sContext->frTextureLayout);
      mPipeline->addDescriptor(sContext->frTextureLayout);
      mPipeline->addDescriptor(sContext->frTextureLayout);
      mPipeline->addPushConstant(VkPushConstantRange{
        VK_SHADER_STAGE_VERTEX_BIT,
        0, sizeof(uint32_t)
      });

      mPipeline->setMultisampleInfo(VkPipelineMultisampleStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        (VkSampleCountFlagBits)sContext->frMsaa, VK_FALSE, 0.0f, VK_NULL_HANDLE,
        VK_FALSE, VK_FALSE
      });

      mPipeline->initialize(sContext->frRenderer, sContext->frSceneRenderPass);
    }

    for (auto shader: mShaders) delete shader;
  }

  void purrPipeline::cleanup() {
    delete mPipeline;
  }

  void purrPipeline::bind() {
    mPipeline->bind(sContext->frActiveCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);
    renderer::bindCamera(mPipeline);
    renderer::bindTransforms(mPipeline);
  }

  void purrPipeline::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

}