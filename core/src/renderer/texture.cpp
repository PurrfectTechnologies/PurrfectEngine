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

  purrTexture::purrTexture(int width, int height, VkFormat format, fr::frCommands *commands):
    mWidth(width), mHeight(height), mFormat(format), mCommands(commands?commands:sContext->frCommands)
  {}

  purrTexture::~purrTexture() {
    cleanup();
  }

  bool purrTexture::initialize(const char *filename, purrSampler *sampler, bool mipmaps, bool color) {
    mMipmaps = mipmaps;
    mColor = color;
    mSampler = sampler;

    if (filename) {
      int c;
      stbi_uc *data = stbi_load(filename, &mWidth, &mHeight, &c, STBI_rgb_alpha);
      if (!data) return false;
      if (!initializeImage(std::vector<uint8_t>(data, data+(mWidth*mHeight*4)))) return false;
      stbi_image_free(data);
    } else {
      if (!initializeImage(std::vector<uint8_t>(0))) return false;
      mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        (VkAccessFlags)0,
        VK_ACCESS_SHADER_READ_BIT,
      });
    }
    return true;
  }

  bool purrTexture::initializeHdr(const char *filename, int channels, purrSampler *sampler, bool mipmaps) {
    if (!filename) return false;

    mColor = true;
    mMipmaps = mipmaps;
    mSampler = sampler;

    int c;
    float *data = stbi_loadf(filename, &mWidth, &mHeight, &c, channels);
    if (!data) return false;
    size_t vecSize = mWidth*mHeight*channels*sizeof(float);
    std::vector<float> vec(vecSize);
    memcpy(vec.data(), data, vecSize);
    if (!initializeImage(vec)) return false;
    stbi_image_free(data);
    return true;
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

  void purrTexture::bind(fr::frPipeline *pipeline, uint32_t set) {
    pipeline->bindDescriptor(sContext->frActiveCmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, set, mDescriptor);
  }

  void purrTexture::setPixels(std::vector<uint8_t> pixels) {
    // size_t maxSize = mWidth * mHeight * PurrfectEngine::Utils::formatToChannels(mFormat);
    // assert(pixels.size() <= maxSize);
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(pixels.size());

    fr::frBuffer *stagingBuffer = new fr::frBuffer();
    stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
      bufferSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
    });
    stagingBuffer->copyData(0, bufferSize, pixels.data());

    mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
      VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT
    });
    mImage->copyFromBuffer(sContext->frRenderer, sContext->frCommands, stagingBuffer, 0);
    if (mMipmaps) mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
    else mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
      VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT
    });

    delete stagingBuffer;
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
    mImage->copyFromBuffer(sContext->frRenderer, sContext->frCommands, stagingBuffer, 0);
    mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
   
    delete stagingBuffer;
  }

  static purrTexture *sBlnk = nullptr;

  purrTexture *purrTexture::getBlankTexture() {
    if (!sBlnk) {
      sBlnk = new purrTexture(512, 512, VK_FORMAT_B8G8R8A8_SRGB);
      sBlnk->initialize();
      std::vector<uint8_t> pixels(512*512*4*sizeof(float), 0xFF);
      sBlnk->setPixels(pixels);
    }
    return sBlnk;
  }

  void purrTexture::cleanupAll() {
    if (sBlnk) delete sBlnk;
  }

  void purrTexture::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  template <typename T>
  inline bool purrTexture::initializeImage(std::vector<T> pixels) {
    if (mImage) cleanup();
    try {
      mImage = new fr::frImage();
      mImage->initialize(sContext->frRenderer, fr::frImage::frImageInfo{
        mWidth, mHeight, 1, mFormat,
        (VkImageUsageFlagBits)((mColor?VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) | (mMipmaps ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0) | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
        true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mColor?VK_IMAGE_ASPECT_COLOR_BIT:VK_IMAGE_ASPECT_DEPTH_BIT, mMipmaps
      });

      if (mSampler) {
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = mSampler->mSampler->get();
        imageInfo.imageView = mImage->getView();
        imageInfo.imageLayout = mColor?VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        mDescriptor = sContext->frTextureDescriptors->allocate(1, sContext->frTextureLayout)[0];
        mDescriptor->update(fr::frDescriptor::frDescriptorWriteInfo{
          0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          &imageInfo, VK_NULL_HANDLE, VK_NULL_HANDLE
        });
      }

      if (pixels.size() > 0) { // Copy
        mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_TRANSFER_BIT,
          0, VK_ACCESS_TRANSFER_WRITE_BIT,
        });

        fr::frBuffer *stagingBuf = new fr::frBuffer();
        stagingBuf->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
          static_cast<uint32_t>(pixels.size()*sizeof(T)), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
        });
        stagingBuf->copyData(0, pixels.size()*sizeof(T), pixels.data());
        mImage->copyFromBuffer(sContext->frRenderer, sContext->frCommands, stagingBuf, 0);
        delete stagingBuf;
        if (mMipmaps) mImage->generateMipmaps(sContext->frRenderer, sContext->frCommands);
        else {
          mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
          });
        }
      } else {
        mImage->transitionLayout(sContext->frRenderer, sContext->frCommands, fr::frImage::frImageTransitionInfo{
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
          0,
          VK_ACCESS_SHADER_READ_BIT,
        });
      }
    } catch (fr::frVulkanException &ex) {
      return false;
    }

    return true;
  }

}