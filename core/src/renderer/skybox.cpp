#include "PurrfectEngine/PurrfectEngine.hpp"

#include <array>

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
    std::array<purrRenderTarget *, 6> targets{};
    std::array<purrTexture *, 6> colors{};
    for (size_t i = 0; i < 6; ++i) {
      targets[i] = new purrRenderTarget();
      targets[i]->initialize({ width, height });
      colors[i] = targets[i]->getColorTarget();
    }

    VkCommandBuffer cmdBuf = sContext->frCommands->beginSingleTime();

    purrMesh *cube = purrMesh::getCubeMesh();

    for (purrRenderTarget *target: targets) {
      target->begin(cmdBuf);
      cube->bind(cmdBuf);
      vkCmdDrawIndexed(cmdBuf, cube->getIndexCount(), 1, 0, 0, 0);
      target->end(cmdBuf);
    }

    purrCubemap *cubemap = new purrCubemap();
    cubemap->initialize(colors);
    
    sContext->frCommands->endSingleTime(sContext->frRenderer, cmdBuf);

    return cubemap;
  }

}