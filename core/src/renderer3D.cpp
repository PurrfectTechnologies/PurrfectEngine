#include "PurrfectEngine/PurrfectEngine.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

namespace PurrfectEngine {

  purrMesh3D::purrMesh3D()
  {}

  purrMesh3D::~purrMesh3D() {
    cleanup();
  }

  bool purrMesh3D::initialize() {
    mVBuffer = new purrBuffer();
    mIBuffer = new purrBuffer();
    return true;
  }

  void purrMesh3D::cleanup() {
    delete mVBuffer;
    delete mIBuffer;
  }

  void purrMesh3D::render(VkCommandBuffer cmdBuf) {
    VkBuffer vbuf = mVBuffer->get();
    VkDeviceSize zero = 0;
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vbuf, &zero);
    vkCmdBindIndexBuffer(cmdBuf, mIBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuf, mIndexCount, 1, 0, 0, 0);
  }

  bool purrMesh3D::copy(std::vector<purrVertex3D> &vertices, std::vector<uint32_t> &indices) {
    uint32_t vertexBufSize = sizeof(purrVertex3D) * vertices.size();
    mIndexCount = static_cast<uint32_t>(indices.size());
    uint32_t indexBufSize = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
    if (mVBuffer->initialize(vertexBufSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true) != VK_SUCCESS) return false;
    if (mIBuffer->initialize(indexBufSize,  VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true) != VK_SUCCESS) return false;

    purrBuffer *staging = new purrBuffer();
    {
      staging->initialize(vertexBufSize, (VkBufferUsageFlagBits)0, false);
      staging->copy(vertices.data(), vertexBufSize, 0);
      if (mVBuffer->copy(staging,    vertexBufSize, 0) != VK_SUCCESS) return false;

      staging->cleanup();
    }

    {
      staging->initialize(indexBufSize, (VkBufferUsageFlagBits)0, false);
      staging->copy(indices.data(), indexBufSize, 0);
      if (mIBuffer->copy(staging,   indexBufSize, 0) != VK_SUCCESS) return false;
    }
    delete staging;

    return true;
  }

  bool load_single_mesh(const aiScene *scene, aiMesh *mesh, purrMesh3D *mesh3D) {
    std::vector<uint32_t> indices{};
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace &face = mesh->mFaces[i];
      if (face.mNumIndices != 3) return false;
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
    }
    std::vector<purrVertex3D> vertices{};
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) vertices.push_back(purrVertex3D{
      glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
      (mesh->HasVertexColors(0)?glm::vec3(
        mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b
      ):glm::vec3(1.0f)),
      (mesh->HasTextureCoords(0)?glm::vec2(
        mesh->mTextureCoords[0][i].x,
        mesh->mTextureCoords[0][i].y
      ):glm::vec2(0.0f)),
      glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
    });
    return mesh3D->initialize() && mesh3D->copy(vertices, indices);
  }

  bool purrMesh3D::loadModel(const char *filename, purrScene *scene, purrObject **root) {
    Assimp::Importer importer;
    const aiScene *paiScene = importer.ReadFile(filename, (aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices));
    if (!paiScene) return false;

    *root = scene->newObject();

    for (uint32_t i = 0; i < paiScene->mNumMeshes; ++i) {
      purrObject *obj = ((paiScene->mNumMeshes>1)?(*root)->newChild():*root);
      purrMesh3D *mesh = new purrMesh3D();
      if (!load_single_mesh(paiScene, paiScene->mMeshes[i], mesh)) { delete mesh; goto load_failed; }
      obj->addComponent(new purrMesh3DComp(mesh));
    }

    return true;
  load_failed:
    scene->removeObject((*root)->getUuid());
    return false;
  }

  purrMesh3DComp::purrMesh3DComp(purrMesh3D *mesh):
    mMesh(mesh)
  {}

  purrMesh3DComp::~purrMesh3DComp() {
    if (mMesh) delete mMesh;
  }

  purrRenderer3D::purrRenderer3D():
    purrRenderer({ new purrOffscreenRendererExt() })
  {}

  void purrRenderer3D::setScene(purrScene *scene) {
    mScene = scene;
  }

  bool purrRenderer3D::update() {
    if (!mScene) return false;

    purrObject *cameraObject = mScene->getCamera();
    purrCameraComp *cameraComp = nullptr;
    purrCamera *camera = nullptr;
    if (cameraObject && (cameraComp = (purrCameraComp*)cameraObject->getComponent("cameraComponent")) && (camera = cameraComp->getCamera())) if (!updateCamera(camera)) return false;

    std::vector<purrObject3D> objects = updateObjects(mScene->getObjects());
    if (!updateObjects(objects)) return false;

    std::vector<purrLight> lights = { { {0.0f,0.0f,4.0f,1.0f}, {1.0f,0.0f,0.0f,1.0f} } };
    if (!updateLights(lights)) return false;

    return true;
  }

  bool purrRenderer3D::createResources() {
    VkPushConstantRange pcRange = {};
    pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pcRange.offset = 0;
    pcRange.size = sizeof(glm::ivec4);

    return ((mRenderTarget->initialize(purrRenderTargetInitInfo{
      VkExtent3D{
        mSwapchainExtent.width,
        mSwapchainExtent.height,
        1
      },
      mSampler, nullptr, nullptr, false
    }) == VK_SUCCESS) && (mPipeline->initialize(purrPipelineInitInfo{
      { { VK_SHADER_STAGE_VERTEX_BIT, "./assets/shaders/scene_v.spv" },
        { VK_SHADER_STAGE_FRAGMENT_BIT, "./assets/shaders/scene_f.spv" }, },
      getSampleCount(),
      purrVertex3D::getBindings(), purrVertex3D::getAttributes(),
      { mSceneLayout }, { pcRange },
      mRenderTarget,
    }) == VK_SUCCESS));
  }

  bool purrRenderer3D::initialize_() {
    { // Create sampler
      mSampler = new purrSampler();
      if (mSampler->initialize(purrSamplerInitInfo{VK_TRUE, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK}) != VK_SUCCESS) return false;
    }
    mRenderTarget = new purrRenderTarget();
    mPipeline = new purrPipeline();

    { // Create scene objects layout
      VkDescriptorSetLayoutBinding bindings[] = {
        VkDescriptorSetLayoutBinding{
          0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE
        },
        VkDescriptorSetLayoutBinding{
          1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE
        },
        VkDescriptorSetLayoutBinding{
          2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE
        },
      };

      VkDescriptorSetLayoutCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      createInfo.pNext = VK_NULL_HANDLE;
      createInfo.flags = 0;
      createInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
      createInfo.pBindings = bindings;

      if (vkCreateDescriptorSetLayout(mDevice, &createInfo, VK_NULL_HANDLE, &mSceneLayout) != VK_SUCCESS) return false;
    }

    { // Create descriptor pool
      VkDescriptorPoolSize poolSizes[] = {
        VkDescriptorPoolSize{
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1
        },
        VkDescriptorPoolSize{
          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2
        }
      };

      VkDescriptorPoolCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      createInfo.pNext = VK_NULL_HANDLE;
      createInfo.flags = 0;
      createInfo.maxSets = 1;
      createInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
      createInfo.pPoolSizes = poolSizes;
      if (vkCreateDescriptorPool(mDevice, &createInfo, VK_NULL_HANDLE, &mDescriptorPool) != VK_SUCCESS) return false;
    }

    { // Allocate scene objects descriptor set
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.pNext = VK_NULL_HANDLE;
      allocInfo.descriptorPool = mDescriptorPool;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = &mSceneLayout;

      if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mSceneSet) != VK_SUCCESS) return false;
    }

    { // Update scene objects descriptor set
      mCameraBuffer = new purrBuffer();
      if (mCameraBuffer->initialize(sizeof(glm::mat4)*2, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, true)) return false;

      VkDescriptorBufferInfo bufferInfo = {};
      bufferInfo.buffer = mCameraBuffer->get();
      bufferInfo.offset = 0;
      bufferInfo.range  = sizeof(glm::mat4)*2;

      VkWriteDescriptorSet writeInfo = {};
      writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeInfo.pNext            = VK_NULL_HANDLE;
      writeInfo.dstSet           = mSceneSet;
      writeInfo.dstBinding       = 0;
      writeInfo.dstArrayElement  = 0;
      writeInfo.descriptorCount  = 1;
      writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeInfo.pImageInfo       = VK_NULL_HANDLE;
      writeInfo.pBufferInfo      = &bufferInfo;
      writeInfo.pTexelBufferView = VK_NULL_HANDLE;

      vkUpdateDescriptorSets(mDevice, 1, &writeInfo, 0, VK_NULL_HANDLE);
    }

    return createResources();
  }

  bool purrRenderer3D::resize_() {
    mRenderTarget->cleanup();
    mPipeline->cleanup();
    return createResources();
  }

  bool purrRenderer3D::render_(VkCommandBuffer cmdBuf) {
    if (!mScene) return false;
    mRenderTarget->begin(cmdBuf);
    mPipeline->bind(cmdBuf);

    vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline->getLayout(), 0, 1, &mSceneSet, 0, VK_NULL_HANDLE);
    size_t idx = 0;
    for (purrObject *object : mScene->getObjects()) {
      purrMesh3DComp *comp = nullptr;
      if (comp = (purrMesh3DComp*)object->getComponent("mesh3DComponent")) {
        glm::ivec4 data = {idx, 0, 0, 0};
        vkCmdPushConstants(cmdBuf, mPipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::ivec4), &data);
        comp->getMesh()->render(cmdBuf);
      }
      ++idx;
    }

    mRenderTarget->end(cmdBuf);

    return true;
  }

  void purrRenderer3D::cleanup_() {
    if (mCameraBuffer)  delete mCameraBuffer;
    if (mObjectsBuffer) delete mObjectsBuffer;
    if (mLightBuffer)   delete mLightBuffer;
    vkDestroyDescriptorSetLayout(mDevice, mSceneLayout, VK_NULL_HANDLE);
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, VK_NULL_HANDLE);

    delete mSampler;
    delete mRenderTarget;
    delete mPipeline;
  }

  purrRendererInitInfo purrRenderer3D::getInitInfo() {
    return purrRendererInitInfo{
      { VK_EXT_DEBUG_UTILS_EXTENSION_NAME }, nullptr, { "VK_LAYER_KHRONOS_validation" }, nullptr, VkPhysicalDeviceFeatures{}, {}, {}, purrRendererSwapchainInfo{ true }
    };
  }

  VkFormat purrRenderer3D::getRenderTargetFormat() {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }

  VkFormat purrRenderer3D::getHdrFormat() {
    return VK_FORMAT_R16G16B16A16_SFLOAT;
  }

  VkFormat purrRenderer3D::getFormat() {
    return VK_FORMAT_R8G8B8A8_SRGB;
  }

  VkSampleCountFlagBits purrRenderer3D::getSampleCount() {
    return VK_SAMPLE_COUNT_1_BIT;
  }

  purrRenderTarget *purrRenderer3D::getRenderTarget() {
    return mRenderTarget;
  }

  std::vector<purrObject3D> purrRenderer3D::updateObjects(std::vector<purrObject*> objects) {
    std::vector<purrObject3D> vec = {};
    for (purrObject *obj: objects) {
      vec.push_back(purrObject3D{obj->getTransform()->getTransform()});
      if (obj->isParent()) {
        std::vector<purrObject3D> vec2 = updateObjects(obj->getChildren());
        vec.insert(vec.end(), vec2.begin(), vec2.end());
      }
    }
    return vec;
  }

  bool purrRenderer3D::updateCamera(purrCamera *camera) {
    if (!camera) return false;
    void *data = malloc(sizeof(glm::mat4)*2);
    glm::mat4 proj = camera->getProjection();
    glm::mat4 view = camera->getView();
    memcpy(data,                            &proj, sizeof(glm::mat4));
    memcpy(((char*)data)+sizeof(glm::mat4), &view, sizeof(glm::mat4));
    purrBuffer *stagingBuf = new purrBuffer();
    if (stagingBuf->initialize(sizeof(glm::mat4)*2, (VkBufferUsageFlagBits)0, false) != VK_SUCCESS) return false;
    stagingBuf->copy(data, sizeof(glm::mat4)*2, 0);
    if (mCameraBuffer->copy(stagingBuf, sizeof(glm::mat4)*2, 0) != VK_SUCCESS) return false;
    delete stagingBuf;
    return true;
  }

  bool purrRenderer3D::updateObjects(std::vector<purrObject3D> objects) {
    if (mObjectsBuffer) delete mObjectsBuffer;
    if (objects.size() <= 0) return true;
    uint32_t objectCount = static_cast<uint32_t>(objects.size());
    uint32_t size = sizeof(purrObject3D)*objectCount;

    mObjectsBuffer = new purrBuffer();
    mObjectsBuffer->initialize(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);

    { // Copy objects
      purrBuffer *stagingBuf = new purrBuffer();
      if (stagingBuf->initialize(size, (VkBufferUsageFlagBits)0, false) != VK_SUCCESS) return false;
      stagingBuf->copy(objects.data(), size, 0);
      if (mObjectsBuffer->copy(stagingBuf, size, 0) != VK_SUCCESS) return false;
      delete stagingBuf;
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = mObjectsBuffer->get();
    bufferInfo.offset = 0;
    bufferInfo.range  = size;

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext            = VK_NULL_HANDLE;
    writeInfo.dstSet           = mSceneSet;
    writeInfo.dstBinding       = 1;
    writeInfo.dstArrayElement  = 0;
    writeInfo.descriptorCount  = 1;
    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeInfo.pImageInfo       = VK_NULL_HANDLE;
    writeInfo.pBufferInfo      = &bufferInfo;
    writeInfo.pTexelBufferView = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(mDevice, 1, &writeInfo, 0, VK_NULL_HANDLE);
    return true;
  }

  bool purrRenderer3D::updateLights(std::vector<purrLight> lights) {
    if (mLightBuffer) delete mLightBuffer;
    if (lights.size() <= 0) return true;
    uint32_t lightCount = static_cast<uint32_t>(lights.size());
    VkDeviceSize size = sizeof(glm::vec4)+sizeof(purrLight)*lightCount;

    mLightBuffer = new purrBuffer();
    mLightBuffer->initialize(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);

    { // Copy light
      purrBuffer *stagingBuf = new purrBuffer();
      if (stagingBuf->initialize(size, (VkBufferUsageFlagBits)0, false) != VK_SUCCESS) return false;
      stagingBuf->copy(&lightCount, sizeof(glm::vec4), 0);
      stagingBuf->copy(lights.data(), size-sizeof(glm::vec4), sizeof(glm::vec4));
      if (mLightBuffer->copy(stagingBuf, size, 0) != VK_SUCCESS) return false;
      delete stagingBuf;
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = mLightBuffer->get();
    bufferInfo.offset = 0;
    bufferInfo.range  = size;

    VkWriteDescriptorSet writeInfo = {};
    writeInfo.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext            = VK_NULL_HANDLE;
    writeInfo.dstSet           = mSceneSet;
    writeInfo.dstBinding       = 2;
    writeInfo.dstArrayElement  = 0;
    writeInfo.descriptorCount  = 1;
    writeInfo.descriptorType   = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeInfo.pImageInfo       = VK_NULL_HANDLE;
    writeInfo.pBufferInfo      = &bufferInfo;
    writeInfo.pTexelBufferView = VK_NULL_HANDLE;

    vkUpdateDescriptorSets(mDevice, 1, &writeInfo, 0, VK_NULL_HANDLE);
    return true;
  }

}