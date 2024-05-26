#ifndef   PURRENGINE_RENDERER_MESH_HPP_
#define   PURRENGINE_RENDERER_MESH_HPP_

namespace PurrfectEngine {

  class purrMesh {
  public:
    purrMesh();
    ~purrMesh();

    void initialize(const char *filepath);
    void initialize(fr::frCommands *commands, std::vector<Vertex3D> vertices, std::vector<uint32_t> indices);
    void cleanup();

    void render(VkCommandBuffer cmdBuf);

    bool isValid() const { return mValid; }

    static void setContext(PurrfectEngineContext *context);

    static purrMesh *getSquareMesh();

    static void cleanupAll();
  private:
    bool mValid = false;

    size_t mIndexCount = 0;
    fr::frBuffer *mVertexBuffer = nullptr;
    fr::frBuffer *mIndexBuffer = nullptr;
  };

  class purrMesh2D {
  public:
    purrMesh2D();
    ~purrMesh2D();

    void initialize(const char *filepath);
    void initialize(fr::frCommands *commands, std::vector<Vertex2D> vertices, std::vector<uint32_t> indices);
    void cleanup();

    void render(VkCommandBuffer cmdBuf);

    static purrMesh2D *getSquareMesh();

    static void cleanupAll();
  private:
    size_t mIndexCount = 0;
    fr::frBuffer *mVertexBuffer = nullptr;
    fr::frBuffer *mIndexBuffer = nullptr;
  };

}

#endif // PURRENGINE_RENDERER_MESH_HPP_