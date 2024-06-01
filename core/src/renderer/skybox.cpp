#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  purrSkybox::purrSkybox()
  {}

  purrSkybox::~purrSkybox() {
    cleanup();
  }

  void purrSkybox::initialize(purrTexture *texture, int width, int height) {
    initialize(textureToCubemap(texture, width, height));
  }

  void purrSkybox::initialize(purrCubemap *cubemap) {
    mSkyboxCubemap = cubemap;
  }
  
  void purrSkybox::cleanup() {

  }

  void purrSkybox::bind(VkCommandBuffer cmdBuf, fr::frPipeline *pipeline) {
    pipeline->bindDescriptor(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, 3, mSkyboxDesc);
  }

  void purrSkybox::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  purrCubemap *purrSkybox::textureToCubemap(purrTexture *texture, int width, int height) {
    std::array<purrTexture *, 6> targets{};
    for (size_t i = 0; i < 6; ++i) {
      targets[i] = new purrTexture(width, height, sContext->frHdrFormat);
      targets[i]->initialize(nullptr, nullptr, false, true);
    }

    purrCubemap *cubemap = new purrCubemap();
    cubemap->initialize(targets);
    return cubemap;
  }

}