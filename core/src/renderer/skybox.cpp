#include "PurrfectEngine/PurrfectEngine.hpp"

#include <array>

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  purrSkybox::purrSkybox()
  {}

  purrSkybox::~purrSkybox() {
    cleanup();
  }

  bool purrSkybox::initialize(purrTexture *texture, int width, int height) {
    purrCubemap *cubemap = textureToCubemap(texture, width, height);
    if (!cubemap) return false;
    return initialize(cubemap);
  }

  bool purrSkybox::initialize(purrCubemap *cubemap) {
    mSkyboxCubemap = cubemap;

    return true;
  }
  
  void purrSkybox::cleanup() {
    if (mSkyboxDesc) delete mSkyboxDesc;
    if (mSkyboxCubemap) delete mSkyboxCubemap;
  }

  void purrSkybox::bind(VkCommandBuffer cmdBuf, fr::frPipeline *pipeline) {
    pipeline->bindDescriptor(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 3, mSkyboxDesc);
  }

  void purrSkybox::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  purrCubemap *purrSkybox::textureToCubemap(purrTexture *texture, int width, int height) {
    std::array<purrRenderTarget *, 6> targets{};
    std::array<purrTexture *, 6> colors{};
    for (size_t i = 0; i < 6; ++i) {
      targets[i] = new purrRenderTarget();
      targets[i]->initialize({ width, height });
      colors[i] = targets[i]->getColorTarget();
    }

    purrMesh *cube = purrMesh::getCubeMesh();

    fr::frPipeline *rectToCubemapPipeline = nullptr;
    { // RectToCubemap shader
      rectToCubemapPipeline = new fr::frPipeline();

      fr::frShader *vertShdr = new fr::frShader();
      vertShdr->initialize(sContext->frRenderer, "../test/shaders/PBR/rectToCubemap_v.spv", VK_SHADER_STAGE_VERTEX_BIT);
      rectToCubemapPipeline->addShader(vertShdr);

      fr::frShader *fragShdr = new fr::frShader();
      fragShdr->initialize(sContext->frRenderer, "../test/shaders/PBR/rectToCubemap_f.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
      rectToCubemapPipeline->addShader(fragShdr);

      rectToCubemapPipeline->setVertexInputState<Vertex3D>();

      rectToCubemapPipeline->setInputAssemblyState(VkPipelineInputAssemblyStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE
      });

      static VkPipelineColorBlendAttachmentState *sColorBlendAttachment = new VkPipelineColorBlendAttachmentState();
      sColorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      sColorBlendAttachment->blendEnable = VK_FALSE;

      rectToCubemapPipeline->setColorBlendState(VkPipelineColorBlendStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        sColorBlendAttachment,
        {0.0f, 0.0f, 0.0f, 0.0f}
      });

      rectToCubemapPipeline->setRasterizationState(VkPipelineRasterizationStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f
      });

      rectToCubemapPipeline->setDynamicState(VkPipelineDynamicStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        0, VK_NULL_HANDLE
      });

      VkViewport *viewport = new VkViewport({ 0,0,static_cast<float>(width),static_cast<float>(height),0.0f,1.0f });
      VkRect2D *scissor = new VkRect2D({ {}, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) } });
      rectToCubemapPipeline->setViewportState(VkPipelineViewportStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        1, viewport, 1, scissor
      });

      rectToCubemapPipeline->setDepthStencilState(VkPipelineDepthStencilStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS,
        VK_FALSE, VK_FALSE,
        {}, {},
        1.0f, 1.0f
      });

      rectToCubemapPipeline->addDescriptor(sContext->frUboLayout);
      rectToCubemapPipeline->addDescriptor(sContext->frTextureLayout);

      rectToCubemapPipeline->setMultisampleInfo(VkPipelineMultisampleStateCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, VK_NULL_HANDLE, 0,
        (VkSampleCountFlagBits)sContext->frMsaa, VK_FALSE, 0.0f, VK_NULL_HANDLE,
        VK_FALSE, VK_FALSE
      });

      rectToCubemapPipeline->initialize(sContext->frRenderer, sContext->frSceneRenderPass);

      delete vertShdr;
      delete fragShdr;
    }

    struct {
      glm::mat4 proj;
      glm::mat4 view;
    } camData;
    camData.proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    std::vector<fr::frBuffer*> camBufs(6);
    std::vector<fr::frDescriptor*> camDescs(6);

    for (size_t i = 0; i < 6; ++i) {
      glm::vec3 rot = glm::vec3(0.0f);
      switch (i) {
      case 0: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); } break;
      case 1: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)); } break;
      case 2: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)); } break;
      case 3: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)); } break;
      case 4: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); } break;
      case 5: { camData.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)); } break;
      default: { return nullptr; }
      }

      camBufs[i] = new fr::frBuffer();
      camBufs[i]->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        sizeof(glm::mat4)*2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, {}
      });
      camBufs[i]->copyData(0, sizeof(glm::mat4)*2, (void*)&camData);

      camDescs[i] = sContext->frSkyboxDescriptors->allocate(1, sContext->frUboLayout)[0];
      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = camBufs[i]->get();
      bufferInfo.offset = 0;
      bufferInfo.range = sizeof(glm::mat4)*2;
      camDescs[i]->update(fr::frDescriptor::frDescriptorWriteInfo{
        0, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_NULL_HANDLE, &bufferInfo, VK_NULL_HANDLE
      });
    }

    VkCommandBuffer cmdBuf = sContext->frCommands->beginSingleTime(); {
      size_t i = 0;
      for (purrRenderTarget *target: targets) {
        target->begin(cmdBuf);
        rectToCubemapPipeline->bind(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS);
        rectToCubemapPipeline->bindDescriptor(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, camDescs[i++]);
        rectToCubemapPipeline->bindDescriptor(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, texture->getDescriptor());
        cube->bind(cmdBuf);
        vkCmdDrawIndexed(cmdBuf, cube->getIndexCount(), 1, 0, 0, 0);
        target->end(cmdBuf);
      }
    } sContext->frCommands->endSingleTime(sContext->frRenderer, cmdBuf);

    purrCubemap *cubemap = new purrCubemap();
    cubemap->initialize(colors, sContext->frHdrFormat);

    for (purrRenderTarget *target: targets) delete target;
    delete rectToCubemapPipeline;

    for (fr::frBuffer *buf: camBufs) delete buf;
    for (fr::frDescriptor *desc: camDescs) delete desc;

    return cubemap;
  }

}