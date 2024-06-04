#ifndef   PURRENGINE_RENDERER_HPP_
#define   PURRENGINE_RENDERER_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fr/fr.hpp>

namespace PurrfectEngine {

  struct Vertex2D {
    glm::vec2 position;
    glm::vec2 uv;

    static VkVertexInputBindingDescription *getBindingDescription() {
      VkVertexInputBindingDescription *bindingDescription = new VkVertexInputBindingDescription();
      bindingDescription->binding = 0;
      bindingDescription->stride = sizeof(Vertex2D);
      bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex2D, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex2D, uv);

      return attributeDescriptions;
    }

    bool operator==(const Vertex2D& other) const {
      return position == other.position && uv == other.uv;
    }
  };

  struct Vertex3D {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;

    static VkVertexInputBindingDescription *getBindingDescription() {
      VkVertexInputBindingDescription *bindingDescription = new VkVertexInputBindingDescription();
      bindingDescription->binding = 0;
      bindingDescription->stride = sizeof(Vertex3D);
      bindingDescription->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
      attributeDescriptions.push_back(VkVertexInputAttributeDescription{
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, position),
      });
      attributeDescriptions.push_back(VkVertexInputAttributeDescription{
        1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, color),
      });
      attributeDescriptions.push_back(VkVertexInputAttributeDescription{
        2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, uv),
      });
      attributeDescriptions.push_back(VkVertexInputAttributeDescription{
        3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, normal),
      });
      return attributeDescriptions;
    }

    bool operator==(const Vertex3D& other) const {
      return position == other.position && color == other.color && uv == other.uv && normal == other.normal;
    }
  };

  class purrPipeline;
  namespace renderer {
    void setContext(PurrfectEngineContext *context);
    void setScene(purrScene *scene);

    void setVSync(bool enabled);
    void setMSAA(MSAA msaa);
    void initialize(std::string title, int width, int height);

    void getSwapchainSize(int *width, int *height);
    void setScenePipeline(purrPipeline *scenePipeline);
    void updateCamera();
    void updateTransforms();

    bool shouldClose();
    bool renderBegin();
    void bindCamera(fr::frPipeline *pipeline);
    void bindTransforms(fr::frPipeline *pipeline);
    void renderScene(purrPipeline *pipeline);
    void render();
    bool present();
    
    void waitIdle();
    void cleanup();
  }

  namespace Utils {
    size_t formatToChannels(VkFormat format);
  }

}

#include "PurrfectEngine/renderer/texture.hpp"
#include "PurrfectEngine/renderer/renderTarget.hpp"
#include "PurrfectEngine/renderer/cubemap.hpp"
#include "PurrfectEngine/renderer/skybox.hpp"
#include "PurrfectEngine/renderer/mesh.hpp"
#include "PurrfectEngine/renderer/pipeline.hpp"

#endif // PURRENGINE_RENDERER_HPP_