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
      nullptr, nullptr, false
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
    mRenderTarget = new purrRenderTarget();
    mPipeline = new purrPipeline();
    return createResources();
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
    delete mRenderTarget;
    delete mPipeline;
  }

  VkFormat purrRenderer3D::getRenderTargetFormat() {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }

  VkSampleCountFlagBits purrRenderer3D::getSampleCount() {
    return VK_SAMPLE_COUNT_1_BIT;
  }

  purrRenderTarget *purrRenderer3D::getRenderTarget() {
    return mRenderTarget;
  }

}