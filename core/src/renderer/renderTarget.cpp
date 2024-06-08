#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  purrRenderTarget::purrRenderTarget(fr::frCommands *commands):
    mCommands(commands?commands:sContext->frCommands)
  {}

  purrRenderTarget::~purrRenderTarget() {
    cleanup();
  }

  void purrRenderTarget::initialize(glm::ivec2 size) {
    mColorTarget = new purrTexture(size.x, size.y, sContext->frHdrFormat, mCommands);
    mColorTarget->initialize(nullptr, purrSampler::getDefault(), false, true);
    
    mDepthTarget = new purrTexture(size.x, size.y, sContext->frDepthFormat, mCommands);
    mDepthTarget->initialize(nullptr, nullptr, false, false);

    mFramebuffer = new fr::frFramebuffer();
    mFramebuffer->initialize(sContext->frRenderer, size.x, size.y, 1, sContext->frSceneRenderPass, { mColorTarget->getImage(), mDepthTarget->getImage() });
    
    mSize = size;
  }

  void purrRenderTarget::cleanup() {
    delete mColorTarget;
    delete mDepthTarget;
    delete mFramebuffer;
  }

  void purrRenderTarget::begin(VkCommandBuffer cmdBuf) {
    std::vector<VkClearValue> clearValues = {{{{0.0f,0.0f,0.0f,1.0f}}}, {{1.0f, 0}}};
    sContext->frSceneRenderPass->begin(cmdBuf, VkExtent2D{static_cast<uint32_t>(mSize.x), static_cast<uint32_t>(mSize.y)}, mFramebuffer, clearValues);
  }

  void purrRenderTarget::end(VkCommandBuffer cmdBuf) {
    sContext->frSceneRenderPass->end(cmdBuf);
  }

  void purrRenderTarget::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

}