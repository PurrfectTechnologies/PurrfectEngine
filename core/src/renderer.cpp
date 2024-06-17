#include "PurrfectEngine/PurrfectEngine.hpp"

#include <inttypes.h>
#include <cstring>
#include <map>
#include <set>

#include <vulkan/vk_enum_string_helper.h>

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

  purrRenderer::purrRenderer()
  {}

  purrRenderer::~purrRenderer() {
    
  }

  bool purrRenderer::initialize(purrWindow *window, purrRendererInitInfo initInfo) {
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
      subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpass.colorAttachmentCount = 0;
      subpass.pColorAttachments = &ref;

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

    mSwapchainInfo = initInfo.swapchainInfo;

    return initialize_();
  }

  bool purrRenderer::render() {
    vkWaitForFences(mDevice, 1, &mFFences[mFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mISemaphs[mFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      recreateSwapchain(mSwapchainInfo);
      return true;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      mError = string_VkResult(result);
      return false;
    }

    vkResetFences(mDevice, 1, &mFFences[mFrame]);

    if (!render_()) return false;

    VkCommandBuffer cmdBuf = mRCommands[mFrame];
    vkResetCommandBuffer(cmdBuf, 0);

    { // Record command buffer
      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = 0;
      beginInfo.pInheritanceInfo = nullptr;

      VkResult result = VK_SUCCESS;
      if ((result = vkBeginCommandBuffer(cmdBuf, &beginInfo)) != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }

      VkClearValue clearColor = {{{0.0f,0.0f,0.0f,1.0f}}};

      VkRenderPassBeginInfo renderPassInfo{};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = mRP;
      renderPassInfo.framebuffer = mSwapchainFramebuffers[imageIndex];
      renderPassInfo.renderArea.offset = {0,0};
      renderPassInfo.renderArea.extent = mSwapchainExtent;
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearColor;

      vkCmdBeginRenderPass(cmdBuf, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);



      vkCmdEndRenderPass(cmdBuf);

      vkEndCommandBuffer(cmdBuf);
    }

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
      } else if (result != VK_SUCCESS) {
        mError = string_VkResult(result);
        return false;
      }
    }

    mFrame = (mFrame+1)%mImageCount;

    return true;
  }

  void purrRenderer::cleanup() {
    cleanup_();

    cleanupSwapchain();

    for (size_t i = 0; i < mImageCount; ++i) {
      vkDestroySemaphore(mDevice, mRSemaphs[i], nullptr);
      vkDestroySemaphore(mDevice, mISemaphs[i], nullptr);
      vkDestroyFence(mDevice, mFFences[i], nullptr);
    }
    vkDestroyCommandPool(mDevice, mRCommandP, nullptr);
    vkDestroyRenderPass(mDevice, mRP, nullptr);
    vkDestroyDevice(mDevice, nullptr);
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
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
      vkDestroyFramebuffer(mDevice, mSwapchainFramebuffers[i], nullptr);
      vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

    mSwapchainImageViews.clear();
    mSwapchainFramebuffers.clear();
  }

  // Renderer 3D (Default renderer):

  purrRenderer3D::purrRenderer3D() {}

  purrRenderer3D::~purrRenderer3D() {

  }

  bool purrRenderer3D::initialize_() {
    return true;
  }

  bool purrRenderer3D::render_() {
    return true;
  }

  void purrRenderer3D::cleanup_() {
    
  }

}