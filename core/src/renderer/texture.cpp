#include "PurrfectEngine/PurrfectEngine.hpp"

#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace PurrfectEngine {

  static PurrfectEngineContext *sContext = nullptr;

  static purrSampler *sDefaultSampler = nullptr;

  purrSampler::purrSampler()
  {}

  purrSampler::~purrSampler() {
    cleanup();
  }

  void purrSampler::initialize(fr::frSampler::frSamplerInfo info) {
    mSampler = new fr::frSampler();
    mSampler->initialize(sContext->frRenderer, info);
  }

  void purrSampler::cleanup() {
    delete mSampler;
  }

  purrSampler *purrSampler::getDefault() {
    if (!sDefaultSampler) {
      sDefaultSampler = new purrSampler();
      sDefaultSampler->initialize(fr::frSampler::frSamplerInfo{});
    }
    return sDefaultSampler;
  }

  void purrSampler::cleanupAll() {
    if (sDefaultSampler) delete sDefaultSampler;
  }

  purrTexture::purrTexture(int width, int height, VkFormat format):
    mWidth(width), mHeight(height), mFormat(format)
  {}

  purrTexture::~purrTexture() {
    cleanup();
  }

  void purrTexture::initialize(const char *filename, purrSampler *sampler, bool mipmaps, bool color) {
    if (mImage) cleanup(); // I don't trust my ability of writing code that won't leak memory (NULL)
    mMipmaps = mipmaps;
    mColor = color;
    mImage = new fr::frImage();
    mImage->initialize(sContext->frRenderer, fr::frImage::frImageInfo{
      mWidth, mHeight, 1, mFormat,
      (VkImageUsageFlagBits)((color?VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) | (mipmaps ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0) | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
      true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      color?VK_IMAGE_ASPECT_COLOR_BIT:VK_IMAGE_ASPECT_DEPTH_BIT, mipmaps
    });
    if (filename) {
      mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, VK_ACCESS_TRANSFER_WRITE_BIT,
      });

      fr::frBuffer *stagingBuf = new fr::frBuffer();
      stagingBuf->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        mWidth * mHeight * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      int w, h, c;
      stbi_uc *data = stbi_load(filename, &w, &h, &c, STBI_rgb_alpha);
      if (w != mWidth || h != mHeight) { cleanup(); return; } // TODO: Add proper handler
      stagingBuf->copyData(0, mWidth*mHeight*4, data);
      stbi_image_free(data);
      delete stagingBuf;
    }
    if (mMipmaps) mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
    else if (!filename) {
      mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
        ((!filename)?VK_IMAGE_LAYOUT_UNDEFINED:VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        ((!filename)?VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT:VK_PIPELINE_STAGE_TRANSFER_BIT),
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        ((!filename)?(VkAccessFlags)0:VK_ACCESS_TRANSFER_WRITE_BIT),
        VK_ACCESS_SHADER_READ_BIT,
      });
    }

    mSampler = sampler;

    if (mSampler) {
      VkDescriptorImageInfo imageInfo = {};
      imageInfo.sampler = mSampler->mSampler->get();
      imageInfo.imageView = mImage->getView();
      imageInfo.imageLayout = color?VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      mDescriptor = sContext->frTextureDescriptors->allocate(1, sContext->frTextureLayout)[0];
      mDescriptor->update(fr::frDescriptor::frDescriptorWriteInfo{
        0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &imageInfo, VK_NULL_HANDLE, VK_NULL_HANDLE
      });
    }
  }

  void purrTexture::cleanup() {
    if (mImage) delete mImage;
    if (mDescriptor) delete mDescriptor;
    mImage = nullptr;
    mDescriptor = nullptr;
  }

  void purrTexture::resize(int width, int height) {
    mWidth = width;
    mHeight = height;
    cleanup();
    initialize(nullptr, mSampler, mMipmaps, mColor);
  }

  void purrTexture::setPixels(std::vector<uint8_t> pixels) {
    size_t maxSize = mWidth * mHeight * PurrfectEngine::Utils::formatToChannels(mFormat);
    assert(pixels.size() <= maxSize);
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(pixels.size());

    fr::frBuffer *stagingBuffer = new fr::frBuffer();
    stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
    });
    stagingBuffer->copyData(0, bufferSize, pixels.data());

    mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      0, VK_ACCESS_TRANSFER_WRITE_BIT
    });
    mImage->copyFromBuffer(sContext->frRenderer, sContext->frCommands, stagingBuffer, bufferSize);
    mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
  }

  void purrTexture::setPixels(uint8_t *pixels, size_t size) {
    size_t maxSize = mWidth * mHeight * PurrfectEngine::Utils::formatToChannels(mFormat);
    assert(size <= maxSize);
    
    fr::frBuffer *stagingBuffer = new fr::frBuffer();
    stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
      size,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
    });
    stagingBuffer->copyData(0, size, pixels);

    mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      0, VK_ACCESS_TRANSFER_WRITE_BIT
    });
    mImage->copyFromBuffer(sContext->frRenderer, sContext->frCommands, stagingBuffer, size);
    mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
  }

  void purrTexture::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

}