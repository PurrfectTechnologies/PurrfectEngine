#ifndef   PURRENGINE_PURRFECTENGINE_HPP_
#define   PURRENGINE_PURRFECTENGINE_HPP_

#include <fr/fr.hpp>
#include "PurrfectEngine/transform.hpp"
#include "PurrfectEngine/camera.hpp"
#include "PurrfectEngine/scene.hpp"
#include "PurrfectEngine/assets.hpp"
namespace PurrfectEngine {

  enum class MSAA {
    None = VK_SAMPLE_COUNT_1_BIT,
    X2   = VK_SAMPLE_COUNT_2_BIT,
    X4   = VK_SAMPLE_COUNT_4_BIT,
    X8   = VK_SAMPLE_COUNT_8_BIT,
    X16  = VK_SAMPLE_COUNT_16_BIT,
    X32  = VK_SAMPLE_COUNT_32_BIT,
    X64  = VK_SAMPLE_COUNT_64_BIT,
  };

  struct PurrfectEngineContext {
    MSAA                            frMsaa = MSAA::None;
    fr::frWindow                   *frWindow = nullptr;
    fr::frRenderer                 *frRenderer = nullptr;
    fr::frSwapchain                *frSwapchain = nullptr;
    fr::frRenderPass               *frRenderPass = nullptr;
    fr::frRenderPass               *frSceneRenderPass = nullptr;
    fr::frPipeline                 *frPipeline = nullptr;
    std::vector<fr::frImage*>       frScImages{};
    std::vector<fr::frFramebuffer*> frFbs{};
    fr::frCommands                 *frCommands = nullptr;
    fr::frDescriptors              *frDescriptors = nullptr;
    fr::frDescriptors              *frTextureDescriptors = nullptr;
    fr::frDescriptorLayout         *frTextureLayout = nullptr;
    fr::frDescriptorLayout         *frUboLayout = nullptr;
    fr::frDescriptorLayout         *frStorageBufLayout = nullptr;
    VkCommandBuffer                 frActiveCmdBuf = VK_NULL_HANDLE;
    VkFormat                        frSceneFormat = VK_FORMAT_UNDEFINED;
    VkFormat                        frDepthFormat = VK_FORMAT_UNDEFINED;

    purrScene *activeScene = nullptr;
  };

}

#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/input.hpp"

#endif // PURRENGINE_PURRFECTENGINE_HPP_