#ifndef   PURRENGINE_RENDERER3D_HPP_
#define   PURRENGINE_RENDERER3D_HPP_

namespace PurrfectEngine {

  struct purrVertex3D {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 UV;
    glm::vec3 normal;

    static std::vector<VkVertexInputAttributeDescription> getAttributes() {
      std::vector<VkVertexInputAttributeDescription> attributes(4);
      attributes[0] = VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(purrVertex3D, position)};
      attributes[1] = VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(purrVertex3D, color)};
      attributes[2] = VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(purrVertex3D, UV)};
      attributes[3] = VkVertexInputAttributeDescription{3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(purrVertex3D, normal)};
      return attributes;
    }

    static std::vector<VkVertexInputBindingDescription> getBindings() {
      VkVertexInputBindingDescription binding = {};
      binding.binding   = 0;
      binding.stride    = sizeof(purrVertex3D);
      binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return {binding};
    }
  };

  class purrMesh3D {
  public:
    purrMesh3D();
    ~purrMesh3D();

    bool initialize();
    void cleanup();

    void render(VkCommandBuffer cmdBuf);

    bool copy(std::vector<purrVertex3D> &vertices,
              std::vector<uint32_t>     &indices);
  private:
    uint32_t mIndexCount = 0;
    purrBuffer *mVBuffer = nullptr;
    purrBuffer *mIBuffer = nullptr;
  };

  class purrModel3D {
  public:
    static bool load(const char *filename, purrMesh3D ***meshes, size_t *meshCount);
  };

  struct purrObject3D {
    glm::mat4 model;
  };

  class purrRenderer3D: public purrRenderer {
  public:
    purrRenderer3D();

    bool setObjectList(std::vector<purrObject3D> objects);
    bool updateCamera(purrCamera *camera);
  private:
    bool createResources();
  private:
    virtual bool initialize_()                   override;
    virtual bool resize_()                       override;
    virtual bool render_(VkCommandBuffer cmdBuf) override;
    virtual void cleanup_()                      override;

    virtual VkFormat getRenderTargetFormat()       override;
    virtual VkFormat getHdrFormat()                override;
    virtual VkFormat getFormat()                   override;
    virtual VkSampleCountFlagBits getSampleCount() override;
    virtual purrRenderTarget *getRenderTarget()    override;
  private:
    VkDescriptorSetLayout mSceneLayout    = VK_NULL_HANDLE; // Layout for objectBuffer and camera UBO.
    VkDescriptorPool      mDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet       mSceneSet       = VK_NULL_HANDLE;

    purrBuffer *mCameraBuffer = nullptr;
    purrBuffer *mObjectsBuffer = nullptr;
  private:
    purrMesh3D       *mMesh = nullptr;
    purrSampler      *mSampler = nullptr;
    purrRenderTarget *mRenderTarget = nullptr;
    purrPipeline     *mPipeline = nullptr;
  };

}

#endif // PURRENGINE_RENDERER3D_HPP_