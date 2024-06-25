#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrRenderer3D::purrRenderer3D():
    purrRenderer({})
  {}

  purrRenderer3D::~purrRenderer3D() {

  }

  bool purrRenderer3D::createResources() {
    return ((mRenderTarget->initialize(purrRenderTargetInitInfo{
      VkExtent3D{
        mSwapchainExtent.width,
        mSwapchainExtent.height,
        1
      },
      mSampler, nullptr, nullptr, false
    }) == VK_SUCCESS) && (mPipeline->initialize(purrPipelineInitInfo{
      { { VK_SHADER_STAGE_VERTEX_BIT, "../assets/shaders/vert.spv" },
        { VK_SHADER_STAGE_FRAGMENT_BIT, "../assets/shaders/frag.spv" }, },
      getSampleCount(),
      {}, {},
      {}, {},
      mRenderTarget,
    }) == VK_SUCCESS));
  }

  bool purrRenderer3D::initialize_() {
    mSampler = new purrSampler();
    mRenderTarget = new purrRenderTarget();
    mPipeline = new purrPipeline();
    return ((mSampler->initialize(purrSamplerInitInfo{
      VK_TRUE,
      VK_FILTER_LINEAR, VK_FILTER_LINEAR,
      VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT,
      VK_BORDER_COLOR_INT_OPAQUE_BLACK
    }) == VK_SUCCESS) && createResources());
  }

  bool purrRenderer3D::resize_() {
    mRenderTarget->cleanup();
    mPipeline->cleanup();
    return createResources();
  }

  bool purrRenderer3D::render_() {
    return true;
  }

  void purrRenderer3D::cleanup_() {
    delete mSampler;
    delete mRenderTarget;
    delete mPipeline;
  }

  VkFormat purrRenderer3D::getRenderTargetFormat() {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }

  VkFormat purrRenderer3D::getHdrFormat() {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }

  VkFormat purrRenderer3D::getFormat() {
    return VK_FORMAT_R8G8B8A8_SRGB;
  }

  VkSampleCountFlagBits purrRenderer3D::getSampleCount() {
    return VK_SAMPLE_COUNT_1_BIT;
  }

  purrRenderTarget *purrRenderer3D::getRenderTarget() {
    return mRenderTarget;
  }

}