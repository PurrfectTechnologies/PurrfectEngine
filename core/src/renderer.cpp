#include "PurrfectEngine/PurrfectEngine.hpp"

#include <inttypes.h>
#include <cstring>
#include <map>
#include <set>

#include <vulkan/vk_enum_string_helper.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace PurrfectEngine {

  purrWindow::purrWindow()
  {}

  purrWindow::~purrWindow() {
    cleanup();
  }

  bool purrWindow::initialize(purrWindowInitInfo initInfo) {
    if (!glfwInit()) return false;
    if (!glfwVulkanSupported()) return false;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return mWindow = glfwCreateWindow(initInfo.width, initInfo.height, initInfo.title, NULL, NULL);
  }

  void purrWindow::cleanup() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
  }

  purrRenderer::purrRenderer(std::vector<purrRendererExt*> extensions):
    purrExtendable<purrRendererExt>(extensions)
  { sInstance = this; }

  purrRenderer::~purrRenderer() {

  }

  bool purrRenderer::initialize(purrWindow *window) {
    purrRendererInitInfo initInfo = getInitInfo();
    mWindow = window;

    { // Instance
      { // Add GLFW extensions
        uint32_t count = 0;
        const char **exts = glfwGetRequiredInstanceExtensions(&count);
        if (!exts) {
          mError = "glfwGetRequiredInstanceExtensions failed!";
          return false;
        }
        while (count > 0) initInfo.extensions.push_back(exts[--count]);
      }

      // bool foundUnsupportedExt = false;
      // { // Check extension support
      //   uint32_t extensionCount = 0;
      //   vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extensionCount, VK_NULL_HANDLE);
      //   std::vector<VkExtensionProperties> properties(extensionCount);
      //   vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extensionCount, properties.data());
      //   std::vector<const char *> extensions = initInfo.extensions;
      //   for (const char *extName: extensions) {
      //     bool found = false;
      //     size_t i = 0;
      //     for (VkExtensionProperties property: properties) {
      //       if (strcmp(extName, property.extensionName) == 0) {
      //         found = true;
      //         break;
      //       } ++i;
      //     }
      //     if (!found) {
      //       foundUnsupportedExt = true;
      //       if (initInfo.unsupportedExtensions) initInfo.unsupportedExtensions->push_back(extName);
      //     }
      //     else if (extensions.size() <= 0) break;
      //   }
      // }

      // bool foundUnsupportedLyr = false;
      // { // Check layer support
      //   uint32_t layerCount = 0;
      //   vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE);
      //   std::vector<VkLayerProperties> properties(layerCount);
      //   vkEnumerateInstanceLayerProperties(&layerCount, properties.data());
      //   std::vector<const char *> layers = initInfo.layers;
      //   for (const char *lyrName: layers) {
      //     bool found = false;
      //     size_t i = 0;
      //     for (VkLayerProperties property: properties) {
      //       if (strcmp(lyrName, property.layerName) == 0) {
      //         found = true;
      //         break;
      //       } ++i;
      //     }
      //     if (!found) {
      //       foundUnsupportedLyr = true;
      //       if (initInfo.unsupportedLayers) initInfo.unsupportedLayers->push_back(lyrName);
      //     }
      //     else if (layers.size() <= 0) break;
      //   }
      // }

      // if (foundUnsupportedExt||foundUnsupportedLyr) {
      //   mError = "There is/are unsupported extension(s) or layer(s)!";
      //   return false;
      // }

      VkApplicationInfo appInfo{};
      appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pApplicationName   = purrApp::get()->getAppName();
      appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
      appInfo.pEngineName        = "PurrfectEngine";
      appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
      appInfo.apiVersion         = VK_API_VERSION_1_1;

      VkInstanceCreateInfo createInfo{};
      createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      createInfo.pApplicationInfo        = &appInfo;
      createInfo.enabledExtensionCount   = static_cast<uint32_t>(initInfo.extensions.size());
      createInfo.ppEnabledExtensionNames = initInfo.extensions.data();
      createInfo.enabledLayerCount       = static_cast<uint32_t>(initInfo.layers.size());
      createInfo.ppEnabledLayerNames     = initInfo.layers.data();

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &mInstance)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    initInfo.deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    { // Physical device
      uint32_t deviceCount = 0;
      vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
      if (deviceCount <= 0) {
        mError = "Failed to find GPUs with Vulkan support!";
        return false;
      }

      std::vector<VkPhysicalDevice> devices(deviceCount);
      vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

      std::multimap<uint32_t, VkPhysicalDevice> candidates{};
      for (VkPhysicalDevice device: devices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        if (!deviceFeatures.geometryShader) continue; // No time to waste in here

        { // Extension support check
          uint32_t extensionCount;
          vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

          std::vector<VkExtensionProperties> availableExtensions(extensionCount);
          vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

          std::set<std::string> requiredExtensions(initInfo.deviceExtensions.begin(), initInfo.deviceExtensions.end());

          for (const auto& extension : availableExtensions) requiredExtensions.erase(extension.extensionName);

          if (!requiredExtensions.empty()) continue;
        }

        uint32_t score = 0;

        { // Score
          switch (deviceProperties.deviceType) {
          case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   score += 50; break;
          case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: score += 25; break;
          case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    score += 10; break;
          case VK_PHYSICAL_DEVICE_TYPE_CPU:            score += 5; break;
          default: break;
          }
        }

        candidates.insert(std::make_pair(score, device));
      }

      if (candidates.rbegin()->first > 0) {
        mGPU = candidates.rbegin()->second;
      } else {
        mError = "Failed to find suitable GPU!";
        return false;
      }
    }

    { // Surface
      VkResult result = VK_SUCCESS;
      if ((result = glfwCreateWindowSurface(mInstance, window->get(), nullptr, &mSurface)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    { // Queues
      uint32_t queueFamilyCount = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &queueFamilyCount, nullptr);
      std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(mGPU, &queueFamilyCount, queueFamilies.data());

      bool graphicsSet = false, presentSet = false;
      for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkQueueFamilyProperties family = queueFamilies[i];

        if (!graphicsSet && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
          graphicsSet = true;
          mGraphicsF = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(mGPU, i, mSurface, &presentSupport);
        if (presentSupport) {
          presentSet = true;
          mPresentF = i;
        }

        if (graphicsSet && presentSet) break;
      }

      if (!graphicsSet || !presentSet) {
        mError = "Failed to find graphics and/or present family!";
        return false;
      }
    }

    { // Logical device
      std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
      std::set<uint32_t> uniqueQueueFamilies = {mGraphicsF, mPresentF};

      float queuePriority = 1.0f;
      for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
      }

      initInfo.deviceFeatures.samplerAnisotropy = VK_TRUE;

      VkDeviceCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
      createInfo.pQueueCreateInfos = queueCreateInfos.data();
      createInfo.pEnabledFeatures = &initInfo.deviceFeatures;
      createInfo.enabledExtensionCount = static_cast<uint32_t>(initInfo.deviceExtensions.size());
      createInfo.ppEnabledExtensionNames = initInfo.deviceExtensions.data();
      createInfo.enabledLayerCount = static_cast<uint32_t>(initInfo.deviceLayers.size());
      createInfo.ppEnabledLayerNames = initInfo.deviceLayers.data();

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateDevice(mGPU, &createInfo, nullptr, &mDevice)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }

      vkGetDeviceQueue(mDevice, mGraphicsF, 0, &mGraphicsQ);
      vkGetDeviceQueue(mDevice, mPresentF, 0, &mPresentQ);
    }

    if (!createSwapchain(initInfo.swapchainInfo)) return false;

    { // Render Pass
      VkAttachmentDescription desc{};
      desc.format         = mSwapchainFormat;
      desc.samples        = VK_SAMPLE_COUNT_1_BIT;
      desc.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
      desc.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
      desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      desc.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

      VkAttachmentReference ref{};
      ref.attachment = 0;
      ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

      VkSubpassDescription subpass{};
      subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 1;
      subpass.pColorAttachments    = &ref;

      VkRenderPassCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
      createInfo.attachmentCount = 1;
      createInfo.pAttachments = &desc;
      createInfo.subpassCount = 1;
      createInfo.pSubpasses = &subpass;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateRenderPass(mDevice, &createInfo, nullptr, &mRP)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    if (!createSwapObjs()) return false;

    { // Command pool
      VkCommandPoolCreateInfo poolInfo{};
      poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      poolInfo.queueFamilyIndex = mGraphicsF;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mRCommandP)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    { // Render commands
      mRCommands = (VkCommandBuffer*)malloc(sizeof(*mRCommands) * mImageCount);

      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = mRCommandP;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = mImageCount;

      VkResult result = VK_SUCCESS;
      if ((result = vkAllocateCommandBuffers(mDevice, &allocInfo, mRCommands)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    { // Synchronization
      mISemaphs = (VkSemaphore*)malloc(sizeof(*mISemaphs) * mImageCount);
      mRSemaphs = (VkSemaphore*)malloc(sizeof(*mRSemaphs) * mImageCount);
      mFFences  = (VkFence*)malloc(sizeof(*mFFences) * mImageCount);

      VkSemaphoreCreateInfo semaphInfo{};
      semaphInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

      VkFenceCreateInfo fenceInfo{};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

      for (size_t i = 0; i < mImageCount; i++) {
        if (vkCreateSemaphore(mDevice, &semaphInfo, nullptr, &mISemaphs[i]) != VK_SUCCESS ||
            vkCreateSemaphore(mDevice, &semaphInfo, nullptr, &mRSemaphs[i]) != VK_SUCCESS ||
            vkCreateFence(mDevice, &fenceInfo, nullptr, &mFFences[i]) != VK_SUCCESS) {
          mError = "Failed to create synchronization objects!";
          return false;
        }
      }
    }

    { // Texture descriptor layout
      VkDescriptorSetLayoutBinding binding{};
      binding.binding = 0;
      binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      binding.descriptorCount = 1;
      binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      binding.pImmutableSamplers = VK_NULL_HANDLE;

      VkDescriptorSetLayoutCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = VK_NULL_HANDLE;
      createInfo.flags = 0;
      createInfo.bindingCount = 1;
      createInfo.pBindings = &binding;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateDescriptorSetLayout(mDevice, &createInfo, VK_NULL_HANDLE, &mTextureLayout)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    { // Pipeline
      std::vector<char> vertShaderCode = Utils::ReadFile("./assets/shaders/shader_v.spv");
      std::vector<char> fragShaderCode = Utils::ReadFile("./assets/shaders/shader_f.spv");

      VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
      if (!vertShaderModule) return false;
      VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);
      if (!fragShaderModule) return false;

      VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
      vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
      vertShaderStageInfo.module = vertShaderModule;
      vertShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
      fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      fragShaderStageInfo.module = fragShaderModule;
      fragShaderStageInfo.pName = "main";

      VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
      vertexInputInfo.vertexBindingDescriptionCount = 0;
      vertexInputInfo.vertexAttributeDescriptionCount = 0;

      VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkPipelineViewportStateCreateInfo viewportState{};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.scissorCount = 1;

      VkPipelineRasterizationStateCreateInfo rasterizer{};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer.lineWidth = 1.0f;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rasterizer.depthBiasEnable = VK_FALSE;

      VkPipelineMultisampleStateCreateInfo multisampling{};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

      VkPipelineColorBlendAttachmentState colorBlendAttachment{};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;

      VkPipelineColorBlendStateCreateInfo colorBlending{};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY;
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f;
      colorBlending.blendConstants[1] = 0.0f;
      colorBlending.blendConstants[2] = 0.0f;
      colorBlending.blendConstants[3] = 0.0f;

      std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
      };

      VkPipelineDynamicStateCreateInfo dynamicState{};
      dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
      dynamicState.pDynamicStates = dynamicStates.data();

      VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &mTextureLayout;
      pipelineLayoutInfo.pushConstantRangeCount = 0;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineL)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }

      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.pDynamicState = &dynamicState;
      pipelineInfo.layout = mPipelineL;
      pipelineInfo.renderPass = mRP;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

      if ((result = vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }

      vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
      vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
    }

    { // Single-time command pool
      VkCommandPoolCreateInfo poolInfo{};
      poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
      poolInfo.queueFamilyIndex = mGraphicsF;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mSCommands)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    { // Texture descriptor pool
      VkDescriptorPoolSize size{
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048
      };

      VkDescriptorPoolCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      createInfo.pNext = VK_NULL_HANDLE;
      createInfo.flags = 0;
      createInfo.maxSets = 2048;
      createInfo.poolSizeCount = 1;
      createInfo.pPoolSizes = &size;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateDescriptorPool(mDevice, &createInfo, VK_NULL_HANDLE, &mTextDescs)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    mSwapchainInfo = initInfo.swapchainInfo;

    mDepthFormat = FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    if (mDepthFormat == VK_FORMAT_UNDEFINED) {
      mError = "Failed to find supported depth format!";
      return false;
    }

    return initialize_();
  }

  bool purrRenderer::render() {
    vkWaitForFences(mDevice, 1, &mFFences[mFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mISemaphs[mFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapchain(mSwapchainInfo);
      return resize_();
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      mError = string_VkResult(result);
      return false;
    }

    vkResetFences(mDevice, 1, &mFFences[mFrame]);

    VkCommandBuffer cmdBuf = mRCommands[mFrame];
    vkResetCommandBuffer(cmdBuf, 0);

    { // Begin command buffer
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = 0;
      beginInfo.pInheritanceInfo = nullptr;

      VkResult result = VK_SUCCESS;
      if ((result = vkBeginCommandBuffer(cmdBuf, &beginInfo)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    for (purrRendererExt *ext: getExtensions()) ext->beginFrame(cmdBuf, imageIndex);
    if (!extsPreUpdate()) return false;
    if (!render_(cmdBuf)) return false;
    if (!extsUpdate()) return false;
    for (purrRendererExt *ext: getExtensions()) ext->endFrame();

    vkEndCommandBuffer(cmdBuf);

    { // Submit
      VkPipelineStageFlags dstStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

      VkSubmitInfo submitInfo{};
      submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.pNext                = VK_NULL_HANDLE;
      submitInfo.waitSemaphoreCount   = 1;
      submitInfo.pWaitSemaphores      = &mISemaphs[mFrame];
      submitInfo.pWaitDstStageMask    = dstStages;
      submitInfo.commandBufferCount   = 1;
      submitInfo.pCommandBuffers      = &cmdBuf;
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores    = &mRSemaphs[mFrame];

      VkResult result = VK_SUCCESS;
      if ((result = vkQueueSubmit(mGraphicsQ, 1, &submitInfo, mFFences[mFrame])) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }

      VkPresentInfoKHR presentInfo{};
      presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      presentInfo.pNext              = VK_NULL_HANDLE;
      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores    = &mRSemaphs[mFrame];
      presentInfo.swapchainCount     = 1;
      presentInfo.pSwapchains        = &mSwapchain;
      presentInfo.pImageIndices      = &imageIndex;
      presentInfo.pResults           = VK_NULL_HANDLE;

      result = vkQueuePresentKHR(mPresentQ, &presentInfo);
      if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain(mSwapchainInfo);
        if (!resize_()) return false;
      } else if (result != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    mFrame = (mFrame+1)%mImageCount;

    return true;
  }

  void purrRenderer::cleanup() {
    extsCleanup();
    cleanup_();

    cleanupSwapchain();

    for (size_t i = 0; i < mImageCount; ++i) {
      vkDestroySemaphore(mDevice, mRSemaphs[i], VK_NULL_HANDLE);
      vkDestroySemaphore(mDevice, mISemaphs[i], VK_NULL_HANDLE);
      vkDestroyFence(mDevice, mFFences[i], VK_NULL_HANDLE);
    }
    vkDestroyDescriptorSetLayout(mDevice, mTextureLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(mDevice, mTextDescs, VK_NULL_HANDLE);
    vkDestroyCommandPool(mDevice, mRCommandP, VK_NULL_HANDLE);
    vkDestroyCommandPool(mDevice, mSCommands, VK_NULL_HANDLE);
    vkDestroyPipeline(mDevice, mPipeline, VK_NULL_HANDLE);
    vkDestroyPipelineLayout(mDevice, mPipelineL, VK_NULL_HANDLE);
    vkDestroyRenderPass(mDevice, mRP, VK_NULL_HANDLE);
    vkDestroyDevice(mDevice, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
    vkDestroyInstance(mInstance, VK_NULL_HANDLE);

    free(mRSemaphs);
    free(mISemaphs);
    free(mFFences);
    free(mRCommands);
  }

  bool purrRenderer::recreateSwapchain(purrRendererSwapchainInfo swapchainInfo) {
    vkDeviceWaitIdle(mDevice);

    int w, h;
    glfwGetWindowSize(mWindow->get(), &w, &h);
    while (w <= 0 || h <= 0) {
      glfwPollEvents();
      glfwGetWindowSize(mWindow->get(), &w, &h);
    }

    cleanupSwapchain();
    return createSwapchain(swapchainInfo) && createSwapObjs();
  }

  bool purrRenderer::createSwapchain(purrRendererSwapchainInfo swapchainInfo) {
    { // Swapchain
      VkSurfaceCapabilitiesKHR capabilities;
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;

      { // Get swapchain info
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mGPU, mSurface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(mGPU, mSurface, &formatCount, nullptr);

        if (formatCount != 0) {
          formats.resize(formatCount);
          vkGetPhysicalDeviceSurfaceFormatsKHR(mGPU, mSurface, &formatCount, formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(mGPU, mSurface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
          presentModes.resize(presentModeCount);
          vkGetPhysicalDeviceSurfacePresentModesKHR(mGPU, mSurface, &presentModeCount, presentModes.data());
        }
      }

      VkSurfaceFormatKHR format = formats[0];
      { // Choose swapchain format
        for (const auto& availableFormat : formats)
          if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            format = availableFormat;
      }
      mSwapchainFormat = format.format;

      VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
      if (!swapchainInfo.VSync) { // Choose swpachain present mode
        for (const auto& availablePresentMode : presentModes)
          if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            presentMode = availablePresentMode;
      }

      VkExtent2D extent = capabilities.currentExtent;
      if (capabilities.currentExtent.width == UINT32_MAX) { // Choose swapchain extent
        int width, height;
        glfwGetFramebufferSize(mWindow->get(), &width, &height);

        extent = {
          static_cast<uint32_t>(width),
          static_cast<uint32_t>(height)
        };

        extent.width = std::clamp(extent.width,   capabilities.minImageExtent.width,  capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
      }
      mSwapchainExtent = extent;

      mImageCount = (capabilities.maxImageCount>0?capabilities.minImageCount+1:std::min(capabilities.minImageCount+1, capabilities.maxImageCount));

      VkSwapchainCreateInfoKHR createInfo{};
      createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface          = mSurface;
      createInfo.minImageCount    = mImageCount;
      createInfo.imageFormat      = format.format;
      createInfo.imageColorSpace  = format.colorSpace;
      createInfo.imageExtent      = extent;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      if (mGraphicsF != mPresentF) {
        uint32_t queueFamilyIndices[] = {mGraphicsF, mPresentF};
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
      }
      createInfo.preTransform = capabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = presentMode;
      createInfo.clipped = VK_TRUE;
      createInfo.oldSwapchain = /*mSwapchain?mSwapchain:*/VK_NULL_HANDLE;

      VkResult result = VK_SUCCESS;
      if ((result = vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapchain)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &mImageCount, nullptr);
    mSwapchainImages.resize(mImageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &mImageCount, mSwapchainImages.data());

    return true;
  }

  bool purrRenderer::createSwapObjs() {
    mSwapchainImageViews.reserve(mImageCount);
    mSwapchainFramebuffers.reserve(mImageCount);
    for (VkImage image: mSwapchainImages) {
      VkImageView view = VK_NULL_HANDLE;
      VkResult result = VK_SUCCESS;

      { // Image view
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = mSwapchainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if ((result = vkCreateImageView(mDevice, &createInfo, nullptr, &view)) != VK_SUCCESS) {
          mError = string_VkResult(result);
          return false;
        }
      }

      mSwapchainImageViews.push_back(view);

      { // Framebuffer
        VkFramebufferCreateInfo createInfo{};
        createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createInfo.renderPass      = mRP;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments    = &view;
        createInfo.width           = mSwapchainExtent.width;
        createInfo.height          = mSwapchainExtent.height;
        createInfo.layers          = 1;

        VkFramebuffer fb = VK_NULL_HANDLE;
        if ((result = vkCreateFramebuffer(mDevice, &createInfo, nullptr, &fb)) != VK_SUCCESS) {
          mError = string_VkResult(result);
          return false;
        }
        mSwapchainFramebuffers.push_back(fb);
      }
    }

    return true;
  }

  void purrRenderer::cleanupSwapchain() {
    for (size_t i = 0; i < mImageCount; ++i) {
      vkDestroyFramebuffer(mDevice, mSwapchainFramebuffers[i], VK_NULL_HANDLE);
      vkDestroyImageView(mDevice, mSwapchainImageViews[i], VK_NULL_HANDLE);
    }
    vkDestroySwapchainKHR(mDevice, mSwapchain, VK_NULL_HANDLE);

    mSwapchainImageViews.clear();
    mSwapchainFramebuffers.clear();
  }

  VkShaderModule purrRenderer::CreateShaderModule(std::vector<char> code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;

    VkResult result = VK_SUCCESS;
    if ((result = vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule)) != VK_SUCCESS) {
      mError = string_VkResult(result);
      return VK_NULL_HANDLE;
    }

    return shaderModule;
  }

  bool purrRenderer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t *result) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(mGPU, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        *result = i;
        return true;
      }
    }

    return false;
  }

  VkFormat purrRenderer::FindSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat c: candidates) {
      VkFormatProperties props{};
      vkGetPhysicalDeviceFormatProperties(mGPU, c, &props);
      if ((tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) ||
          (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)) return c;
    }
    return VK_FORMAT_UNDEFINED;
  }

  VkCommandBuffer purrRenderer::BeginSingleTime() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = mSCommands;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuf;
    vkAllocateCommandBuffers(mDevice, &allocInfo, &cmdBuf);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    return cmdBuf;
  }

  void purrRenderer::EndSingleTime(VkCommandBuffer cmdBuf) {
    vkEndCommandBuffer(cmdBuf);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuf;

    vkQueueSubmit(mGraphicsQ, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(mGraphicsQ);

    vkFreeCommandBuffers(mDevice, mSCommands, 1, &cmdBuf);
  }

  purrOffscreenRendererExt::purrOffscreenRendererExt()
  {}

  purrOffscreenRendererExt::~purrOffscreenRendererExt() {

  }

  bool purrOffscreenRendererExt::initialize() {
    return true;
  }

  bool purrOffscreenRendererExt::preUpdate() {
    return true;
  }

  bool purrOffscreenRendererExt::update() {
    purrRenderer *renderer = purrRenderer::getInstance();

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer->mRP;
    renderPassInfo.framebuffer = renderer->mSwapchainFramebuffers[mImageIndex];
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = renderer->mSwapchainExtent;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(mCmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(mCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->mPipeline);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(renderer->mSwapchainExtent.width);
    viewport.height   = static_cast<float>(renderer->mSwapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(mCmdBuf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderer->mSwapchainExtent;
    vkCmdSetScissor(mCmdBuf, 0, 1, &scissor);

    renderer->getRenderTarget()->getColor()->bind(mCmdBuf, renderer->mPipelineL, 0);
    vkCmdDraw(mCmdBuf, 6, 1, 0, 0);

    vkCmdEndRenderPass(mCmdBuf);

    return true;
  }

  void purrOffscreenRendererExt::cleanup() {

  }

  purrBuffer::purrBuffer()
  {}

  purrBuffer::~purrBuffer() {
    cleanup();
  }

  VkResult purrBuffer::initialize(VkDeviceSize size, VkBufferUsageFlagBits usage, bool local) {
    purrRenderer *renderer = purrRenderer::getInstance();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext                 = VK_NULL_HANDLE;
    bufferInfo.flags                 = 0;
    bufferInfo.size                  = size;
    bufferInfo.usage                 = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage;
    bufferInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;

    VkResult result = VK_SUCCESS;
    if ((result = vkCreateBuffer(renderer->mDevice, &bufferInfo, VK_NULL_HANDLE, &mBuffer)) != VK_SUCCESS) return result;

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(renderer->mDevice, mBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!renderer->FindMemoryType(memRequirements.memoryTypeBits, (local?VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT:VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), &allocInfo.memoryTypeIndex)) return VK_ERROR_INITIALIZATION_FAILED;

    if ((result = vkAllocateMemory(renderer->mDevice, &allocInfo, nullptr, &mMemory)) != VK_SUCCESS) return result;
    vkBindBufferMemory(renderer->mDevice, mBuffer, mMemory, 0);
    return VK_SUCCESS;
  }

  void purrBuffer::cleanup() {
    purrRenderer *renderer = purrRenderer::getInstance();
    vkDestroyBuffer(renderer->mDevice, mBuffer, VK_NULL_HANDLE);
    vkFreeMemory(renderer->mDevice, mMemory, VK_NULL_HANDLE);
  }

  void purrBuffer::copy(void *data, VkDeviceSize size, VkDeviceSize offset) {
    purrRenderer *renderer = purrRenderer::getInstance();
    void *pData = nullptr;
    vkMapMemory(renderer->mDevice, mMemory, offset, size, 0, &pData);
    memcpy(pData, data, size);
    vkUnmapMemory(renderer->mDevice, mMemory);
  }

  VkResult purrBuffer::copy(purrBuffer *src, VkDeviceSize size, VkDeviceSize offset) {
    purrRenderer *renderer = purrRenderer::getInstance();
    VkCommandBuffer cmdBuf = renderer->BeginSingleTime();

    VkBufferCopy region = {};
    region.srcOffset = offset;
    region.dstOffset = offset;
    region.size = size;

    vkCmdCopyBuffer(cmdBuf, src->mBuffer, mBuffer, 1, &region);

    renderer->EndSingleTime(cmdBuf);

    return VK_SUCCESS;
  }

  purrImage::purrImage()
  {}

  purrImage::~purrImage() {
    cleanup();
  }

  VkResult purrImage::initialize(purrImageInitInfo initInfo) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext                 = VK_NULL_HANDLE;
    imageInfo.flags                 = 0;
    imageInfo.imageType             = initInfo.type;
    imageInfo.format                = initInfo.format;
    imageInfo.extent                = initInfo.extent;
    imageInfo.mipLevels             = 1; // TODO: Implement bitmaps
    imageInfo.arrayLayers           = initInfo.arrayLayers;
    imageInfo.samples               = initInfo.samples;
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = (((initInfo.aspect==VK_IMAGE_ASPECT_COLOR_BIT)?VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = VK_NULL_HANDLE;
    imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    purrRenderer *renderer = purrRenderer::getInstance();

    VkResult result = VK_SUCCESS;
    if ((result = vkCreateImage(renderer->mDevice, &imageInfo, VK_NULL_HANDLE, &mImage)) != VK_SUCCESS) return result;

    VkMemoryRequirements memRequirements{};
    vkGetImageMemoryRequirements(renderer->mDevice, mImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    if (!renderer->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex)) return VK_INCOMPLETE;

    if ((result = vkAllocateMemory(renderer->mDevice, &allocInfo, nullptr, &mMemory)) != VK_SUCCESS) return result;

    vkBindImageMemory(renderer->mDevice, mImage, mMemory, 0);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext            = VK_NULL_HANDLE;
    viewInfo.flags            = 0;
    viewInfo.image            = mImage;
    viewInfo.viewType         = initInfo.viewType;
    viewInfo.format           = initInfo.format;
    viewInfo.components.r     = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g     = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b     = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a     = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange = {
      (VkImageAspectFlags)initInfo.aspect, 0, 1, 0, initInfo.arrayLayers
    };

    return vkCreateImageView(renderer->mDevice, &viewInfo, VK_NULL_HANDLE, &mView);
  }

  void purrImage::cleanup() {
    VkDevice device = purrRenderer::getInstance()->mDevice;
    if (mImage)  vkDestroyImage(device, mImage, VK_NULL_HANDLE);
    if (mView)   vkDestroyImageView(device, mView, VK_NULL_HANDLE);
    if (mMemory) vkFreeMemory(device, mMemory, VK_NULL_HANDLE);
  }

  VkResult purrImage::copyFromBuffer(purrBuffer *src) {
    VkResult result = VK_SUCCESS;
    return result;
  }

  VkResult purrImage::copyToBuffer(purrBuffer *dst) {
    VkResult result = VK_SUCCESS;
    return result;
  }

  VkResult purrImage::copyFromImage(purrImage *src) {
    VkResult result = VK_SUCCESS;
    return result;
  }

  VkResult purrImage::transitionLayout(VkImageLayout layout, VkPipelineStageFlagBits stage, VkAccessFlags accessFlag) {
    VkResult result = VK_SUCCESS;
    return result;
  }

  purrSampler::purrSampler()
  {}

  purrSampler::~purrSampler() {
    cleanup();
  }

  VkResult purrSampler::initialize(purrSamplerInitInfo initInfo) {
    purrRenderer *renderer = purrRenderer::getInstance();
    VkSamplerCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext                   = VK_NULL_HANDLE;
    createInfo.flags                   = 0;
    createInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias              = 0.0f;
    createInfo.compareEnable           = VK_FALSE;
    createInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    createInfo.minLod                  = 0.0f;
    createInfo.maxLod                  = 0.0f;
    createInfo.unnormalizedCoordinates = VK_FALSE;
    createInfo.magFilter               = initInfo.magFilter;
    createInfo.minFilter               = initInfo.minFilter;
    createInfo.addressModeU            = initInfo.addressModeU;
    createInfo.addressModeV            = initInfo.addressModeV;
    createInfo.addressModeW            = initInfo.addressModeW;
    createInfo.borderColor             = initInfo.borderColor;
    createInfo.anisotropyEnable        = initInfo.anisotropyEnable;

    if (createInfo.anisotropyEnable) {
      VkPhysicalDeviceProperties properties{};
      vkGetPhysicalDeviceProperties(renderer->mGPU, &properties);
      createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    } else createInfo.maxAnisotropy = 0.0f;

    return vkCreateSampler(renderer->mDevice, &createInfo, VK_NULL_HANDLE, &mSampler);
  }

  void purrSampler::cleanup() {
    purrRenderer *renderer = purrRenderer::getInstance();
    vkDestroySampler(renderer->mDevice, mSampler, VK_NULL_HANDLE);
  }

  purrTexture::purrTexture() {

  }

  void purrTexture::bind(VkCommandBuffer cmdBuf, VkPipelineLayout layout, uint32_t set) {
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &mSet, 0, NULL);
  }

  void purrTexture::bind(VkCommandBuffer cmdBuf, purrPipeline *pipeline, uint32_t set) {
    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), set, 1, &mSet, 0, NULL);
  }

  VkResult purrTexture::createTextureSet() {
    purrRenderer *renderer = purrRenderer::getInstance();
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = VK_NULL_HANDLE;
    allocInfo.descriptorPool = renderer->mTextDescs;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &renderer->mTextureLayout;

    return vkAllocateDescriptorSets(renderer->mDevice, &allocInfo, &mSet);
  }

  void purrTexture::writeImageToSet(VkSampler sampler) {
    purrRenderer *renderer = purrRenderer::getInstance();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler;
    imageInfo.imageView = mImage->getView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeInfo{};
    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext            = VK_NULL_HANDLE;
    writeInfo.dstSet           = mSet;
    writeInfo.dstBinding       = 0;
    writeInfo.dstArrayElement  = 0;
    writeInfo.descriptorCount  = 1;
    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.pImageInfo       = &imageInfo;
    writeInfo.pBufferInfo      = VK_NULL_HANDLE;
    writeInfo.pTexelBufferView = VK_NULL_HANDLE;
    vkUpdateDescriptorSets(renderer->mDevice, 1, &writeInfo, 0, VK_NULL_HANDLE);
  }

  purrTexture2D::purrTexture2D():
    purrTexture()
  {}

  purrTexture2D::~purrTexture2D() {
    cleanup();
  }

  VkResult purrTexture2D::initialize(purrTexture2DInitInfo initInfo) {
    mImage = new purrImage();
    VkResult result = VK_SUCCESS;
    if ((result = mImage->initialize(purrImageInitInfo{
      initInfo.format,
      VkExtent3D{
        static_cast<uint32_t>(initInfo.width),
        static_cast<uint32_t>(initInfo.height),
        1
      },
      1,
      VK_IMAGE_TYPE_2D,
      VK_IMAGE_VIEW_TYPE_2D,
      (initInfo.color?VK_IMAGE_ASPECT_COLOR_BIT:VK_IMAGE_ASPECT_DEPTH_BIT),
      initInfo.samples
    })) != VK_SUCCESS) return result;
    if ((result = mImage->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT)) != VK_SUCCESS) return result;
    if ((result = createTextureSet()) != VK_SUCCESS) return result;
    writeImageToSet(initInfo.sampler->getSampler());

    return VK_SUCCESS;
  }

  void purrTexture2D::cleanup() {
    delete mImage;
  }

  purrRenderTarget::purrRenderTarget()
  {}

  purrRenderTarget::~purrRenderTarget() {
    cleanup();
  }

  VkResult purrRenderTarget::initialize(purrRenderTargetInitInfo initInfo) {
    mInitInfo = initInfo;
    purrRenderer *renderer = purrRenderer::getInstance();

    VkResult result = VK_SUCCESS;
    if (!mInitInfo.colorImage) {
      mInitInfo.colorImage = new purrTexture2D();
      if ((result = ((purrTexture2D*)mInitInfo.colorImage)->initialize(purrTexture2DInitInfo{
        static_cast<int>(mInitInfo.extent.width),
        static_cast<int>(mInitInfo.extent.height),
        renderer->getRenderTargetFormat(),
        true,
        renderer->getSampleCount(),
        mInitInfo.sampler
      })) != VK_SUCCESS) return result;
    }

    if (mInitInfo.depth && !mInitInfo.depthImage) {
      mInitInfo.depthImage = new purrTexture2D();
      if ((result = ((purrTexture2D*)mInitInfo.depthImage)->initialize(purrTexture2DInitInfo{
        static_cast<int>(mInitInfo.extent.width),
        static_cast<int>(mInitInfo.extent.height),
        renderer->mDepthFormat,
        false,
        renderer->getSampleCount(),
        mInitInfo.sampler
      })) != VK_SUCCESS) return result;
    }

    std::vector<VkAttachmentDescription> descs{};
    descs.push_back(VkAttachmentDescription{0,
      renderer->getRenderTargetFormat(),
      renderer->getSampleCount(),
      VK_ATTACHMENT_LOAD_OP_CLEAR,
      VK_ATTACHMENT_STORE_OP_STORE,
      VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      VK_ATTACHMENT_STORE_OP_DONT_CARE,
      VK_IMAGE_LAYOUT_UNDEFINED,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    });

    if (mInitInfo.depth) {
      descs.push_back(VkAttachmentDescription{0,
        renderer->mDepthFormat,
        renderer->getSampleCount(),
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
      });
    }

    VkAttachmentReference ref{};
    ref.attachment      = 0;
    ref.layout          = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &ref;
    if (mInitInfo.depth) subpass.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = static_cast<uint32_t>(descs.size());
    createInfo.pAttachments = descs.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    if ((result = vkCreateRenderPass(renderer->mDevice, &createInfo, nullptr, &mRenderPass)) != VK_SUCCESS) return result;

    VkImageView *attachments = (VkImageView*)malloc(sizeof(VkImageView)*(mInitInfo.depth+1));
    attachments[0] = mInitInfo.colorImage->getView();
    if (mInitInfo.depth) attachments[1] = mInitInfo.depthImage->getView();

    VkFramebufferCreateInfo fbInfo{};
    fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.renderPass      = mRenderPass;
    fbInfo.attachmentCount = mInitInfo.depth+1;
    fbInfo.pAttachments    = attachments;
    fbInfo.width           = mInitInfo.extent.width;
    fbInfo.height          = mInitInfo.extent.height;
    fbInfo.layers          = 1;

    result = vkCreateFramebuffer(renderer->mDevice, &fbInfo, nullptr, &mFramebuffer);
    free(attachments);
    return result;
  }

  void purrRenderTarget::cleanup() {
    purrRenderer *renderer = purrRenderer::getInstance();
    if (mInitInfo.colorImage) delete mInitInfo.colorImage;
    if (mInitInfo.depthImage) delete mInitInfo.depthImage;
    if (mFramebuffer) vkDestroyFramebuffer(renderer->mDevice, mFramebuffer, VK_NULL_HANDLE);
    if (mRenderPass) vkDestroyRenderPass(renderer->mDevice, mRenderPass, VK_NULL_HANDLE);
  }

  void purrRenderTarget::begin(VkCommandBuffer cmdBuf) {
    purrRenderer *renderer = purrRenderer::getInstance();

    VkClearValue *clearValues = (VkClearValue*)malloc(sizeof(VkClearValue)*(mInitInfo.depth+1));
    clearValues[0] = (VkClearValue{{{0.0f,0.0f,0.0f,1.0}}});
    if (mInitInfo.depth) clearValues[1] = (VkClearValue{{0.0f,1.0f}});

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = mRenderPass;
    renderPassInfo.framebuffer = mFramebuffer;
    renderPassInfo.renderArea.offset = {0,0};
    renderPassInfo.renderArea.extent = VkExtent2D{mInitInfo.extent.width, mInitInfo.extent.height};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(mInitInfo.extent.width);
    viewport.height   = static_cast<float>(mInitInfo.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width  = mInitInfo.extent.width;
    scissor.extent.height = mInitInfo.extent.height;
    vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

    free(clearValues);
  }

  void purrRenderTarget::end(VkCommandBuffer cmdBuf) {
    vkCmdEndRenderPass(cmdBuf);
  }

  purrPipeline::purrPipeline()
  {}

  purrPipeline::~purrPipeline() {
    cleanup();
  }

  VkResult purrPipeline::initialize(purrPipelineInitInfo initInfo) {
    purrRenderer *renderer = purrRenderer::getInstance();

    std::vector<VkShaderModule> modules{};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
    for (std::pair<VkShaderStageFlagBits, const char*> kv: initInfo.shaders) {
      std::vector<char> code = Utils::ReadFile(kv.second);
      modules.push_back(renderer->CreateShaderModule(code));

      shaderStages.push_back(VkPipelineShaderStageCreateInfo{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, VK_NULL_HANDLE, 0,
        kv.first,
        modules[modules.size()-1],
        "main"
      });
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(initInfo.bindings.size());
    vertexInputInfo.pVertexBindingDescriptions      = initInfo.bindings.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(initInfo.attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions    = initInfo.attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = initInfo.samples;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(initInfo.layouts.size());
    pipelineLayoutInfo.pSetLayouts = initInfo.layouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(initInfo.pushConstants.size());
    pipelineLayoutInfo.pPushConstantRanges = initInfo.pushConstants.data();

    VkResult result = VK_SUCCESS;
    if ((result = vkCreatePipelineLayout(renderer->mDevice, &pipelineLayoutInfo, nullptr, &mLayout)) != VK_SUCCESS) return result;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = mLayout;
    pipelineInfo.renderPass = initInfo.target->mRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    mInitInfo = initInfo;

    result = vkCreateGraphicsPipelines(renderer->mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline);
    for (VkShaderModule m: modules) vkDestroyShaderModule(renderer->mDevice, m, nullptr);
    return result;
  }

  void purrPipeline::cleanup() {
    purrRenderer *renderer = purrRenderer::getInstance();
    if (mLayout) vkDestroyPipelineLayout(renderer->mDevice, mLayout, VK_NULL_HANDLE);
    if (mPipeline) vkDestroyPipeline(renderer->mDevice, mPipeline, VK_NULL_HANDLE);
  }

  void purrPipeline::bind(VkCommandBuffer cmdBuf) {
    vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
  }

}