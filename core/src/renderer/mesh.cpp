#include "PurrfectEngine/PurrfectEngine.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | /*aiProcess_FlipUVs | aiProcess_MakeLeftHanded | */aiProcess_JoinIdenticalVertices)

namespace PurrfectEngine {

  struct mesh {
    uint32_t indexCount; // size of indices in this mesh.
    uint32_t vertex;     // first # of first vertex of this mesh.
    uint32_t index;      // first # of first index of this mesh.
  };

  namespace Utils {

    static bool loadMesh(const char *filename, std::vector<Vertex3D> &vertices, std::vector<uint32_t> &indices, std::vector<mesh> &meshes, const char **error) {
      Assimp::Importer importer;
      const aiScene *scene = importer.ReadFile(filename, ASSIMP_LOAD_FLAGS);
      if (!scene) {
        *error = importer.GetErrorString();
        return false;
      }

      meshes.resize(scene->mNumMeshes);
      uint32_t vertex = 0;
      uint32_t index = 0;

      const aiVector3D zero(0.0f, 0.0f, 0.0f);
      for (uint32_t i = 0; i < meshes.size(); ++i) {
        const aiMesh *mesh = scene->mMeshes[i];

        meshes[i].indexCount = mesh->mNumFaces * 3;
        meshes[i].index      = index;
        meshes[i].vertex     = vertex;

        vertex += mesh->mNumVertices;
        index  += meshes[i].indexCount;
        vertices.reserve(mesh->mNumVertices);
        indices.reserve(meshes[i].indexCount);

        for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
          const aiVector3D &pos    = mesh->mVertices[i];
          const aiVector3D &color  = mesh->HasVertexColors(0) ?
                                      aiVector3D(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b) : 
                                      aiVector3D(1.0f, 1.0f, 1.0f);
          const aiVector3D &UV     = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : zero;
          const aiVector3D &normal = mesh->mNormals[i];

          vertices.push_back(Vertex3D({
            glm::vec3(pos.x, pos.y, pos.z),
            glm::vec3(color.x, color.y, color.z),
            glm::vec2(UV.x, UV.y),
            glm::vec3(normal.x, normal.y, normal.z)
          }));
        }

        for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
          const aiFace face = mesh->mFaces[i];
          assert(face.mNumIndices == 3); // TODO: Maybe support meshes with non 3-vertex faces?
          indices.push_back(face.mIndices[0]);
          indices.push_back(face.mIndices[1]);
          indices.push_back(face.mIndices[2]);
        }
      }

      return true;
    }

  }

  PurrfectEngineContext *sContext = nullptr;

  static purrMesh *sSquareMesh = nullptr;

  purrMesh::purrMesh():
    mValid(false)
  {}

  purrMesh::~purrMesh() {
    cleanup();
  }

  void purrMesh::initialize(const char *filepath) {
    std::vector<Vertex3D> vertices{};
    std::vector<uint32_t> indices{};
    std::vector<mesh>     meshes{};

    const char *error;
    if (!Utils::loadMesh(filepath, vertices, indices, meshes, &error)) {
      fprintf(stderr, "[purrMesh]: Failed to load model %s: %s\n", filepath, error);
    } else initialize(sContext->frCommands, vertices, indices);
  }
  
  void purrMesh::initialize(fr::frCommands *commands, std::vector<Vertex3D> vertices, std::vector<uint32_t> indices) {
    {
      VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

      fr::frBuffer *stagingBuffer = new fr::frBuffer();
      stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      stagingBuffer->copyData(0, bufferSize, vertices.data());

      mVertexBuffer = new fr::frBuffer();
      mVertexBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      mVertexBuffer->copyFromBuffer(sContext->frRenderer, commands, stagingBuffer, bufferSize);

      delete stagingBuffer;
    }

    {
      VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

      fr::frBuffer *stagingBuffer = new fr::frBuffer();
      stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      stagingBuffer->copyData(0, bufferSize, indices.data());

      mIndexBuffer = new fr::frBuffer();
      mIndexBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      mIndexBuffer->copyFromBuffer(sContext->frRenderer, commands, stagingBuffer, bufferSize);
      mIndexCount = indices.size();

      delete stagingBuffer;
    }

    mValid = true;
  }

  void purrMesh::cleanup() {
    if (!mValid) return;
    delete mVertexBuffer;
    delete mIndexBuffer;
    mValid = false;
  }

  void purrMesh::render(VkCommandBuffer cmdBuf) {
    if (!mValid) return;
    VkDeviceSize offsets[] = {0};
    VkBuffer vbufs[] = { mVertexBuffer->get() };
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vbufs, offsets);
    vkCmdBindIndexBuffer(cmdBuf, mIndexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuf, mIndexCount, 1, 0, 0, 0);
  }

  void purrMesh::setContext(PurrfectEngineContext *context) {
    sContext = context;
  }

  purrMesh *purrMesh::getSquareMesh() {
    if (!sSquareMesh) {
      sSquareMesh = new purrMesh();
      sSquareMesh->initialize(
        sContext->frCommands,
        {
          { {-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
          { { 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} },
          { { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 0.0f} },
          { {-1.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 0.0f} }
        },
        { 0, 2, 1, 0, 3, 2 }
      );
    }
    return sSquareMesh;
  }

  void purrMesh::cleanupAll() {
    if (sSquareMesh) delete sSquareMesh;
  }

  // 2D

  static purrMesh2D *sSquareMesh2D = nullptr;

  purrMesh2D::purrMesh2D()
  {}

  purrMesh2D::~purrMesh2D() {
    cleanup();
  }

  void purrMesh2D::initialize(const char *filepath) {
    // TODO: Implement
  }
  
  void purrMesh2D::initialize(fr::frCommands *commands, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices) {
    {
      VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

      fr::frBuffer *stagingBuffer = new fr::frBuffer();
      stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      stagingBuffer->copyData(0, bufferSize, vertices.data());

      mVertexBuffer = new fr::frBuffer();
      mVertexBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      mVertexBuffer->copyFromBuffer(sContext->frRenderer, commands, stagingBuffer, bufferSize);

      delete stagingBuffer;
    }

    {
      VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

      fr::frBuffer *stagingBuffer = new fr::frBuffer();
      stagingBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      stagingBuffer->copyData(0, bufferSize, indices.data());

      mIndexBuffer = new fr::frBuffer();
      mIndexBuffer->initialize(sContext->frRenderer, fr::frBuffer::frBufferInfo{
        bufferSize,
        (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, {}
      });
      mIndexBuffer->copyFromBuffer(sContext->frRenderer, commands, stagingBuffer, bufferSize);
      mIndexCount = indices.size();

      delete stagingBuffer;
    }
  }

  void purrMesh2D::cleanup() {
    delete mVertexBuffer;
    delete mIndexBuffer;
  }

  void purrMesh2D::render(VkCommandBuffer cmdBuf) {
    VkDeviceSize offsets[] = {0};
    VkBuffer vbufs[] = { mVertexBuffer->get() };
    vkCmdBindVertexBuffers(cmdBuf, 0, 1, vbufs, offsets);
    vkCmdBindIndexBuffer(cmdBuf, mIndexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(cmdBuf, mIndexCount, 1, 0, 0, 0);
  }

  purrMesh2D *purrMesh2D::getSquareMesh() {
    if (!sSquareMesh2D) {
      sSquareMesh2D = new purrMesh2D();
      sSquareMesh2D->initialize(
        sContext->frCommands,
        { {{-1.0f, -1.0f}, {1.0f, 0.0f}},
          {{ 1.0f, -1.0f}, {0.0f, 0.0f}},
          {{ 1.0f,  1.0f}, {0.0f, 1.0f}},
          {{-1.0f,  1.0f}, {1.0f, 1.0f}}},
        { 0, 1, 2, 2, 3, 0 }
      );
    }
    return sSquareMesh2D;
  }

  void purrMesh2D::cleanupAll() {
    delete sSquareMesh2D;
  }

}