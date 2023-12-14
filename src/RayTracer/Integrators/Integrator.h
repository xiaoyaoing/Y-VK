#pragma once
#include "Core/RenderContext.h"
#include "shaders/Raytracing/commons.h"


struct RTModel
{
    std::vector<glm::vec3> position;
    
};

struct RTPrimitive
{
    uint32_t materialIndex;
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
    glm::mat4 worldMatrix;
};

struct BlasInput
{
    std::vector<VkAccelerationStructureGeometryKHR> geometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range;
};


using TlasInput  = VkAccelerationStructureInstanceKHR;


static VkTransformMatrixKHR toVkTransformMatrix(const glm::mat4 & matrix)
{
    VkTransformMatrixKHR transformMatrix{};
    const  auto tMatrix = glm::transpose(matrix);

    memcpy(transformMatrix.matrix, &tMatrix, sizeof(transformMatrix.matrix));
    return transformMatrix;
}


class Integrator
{
public:
    Integrator(Device & device);
    virtual  ~Integrator();

    virtual  void init(Scene & scene);
    virtual  void initScene(Scene & scene);
    virtual  void render(RenderGraph & renderGraph) = 0;
    virtual  void updateGui();
    virtual  void destroy();
    virtual  void update();
    virtual void buildBLAS();
    virtual void buildTLAS();

    Accel createAccel(VkAccelerationStructureCreateInfoKHR& accel);
    void setCamera(std::shared_ptr<Camera> camera);    
protected:
    SceneUbo sceneUbo;
    
    std::unique_ptr<Buffer> vertexBuffer{nullptr};
    std::unique_ptr<Buffer> normalBuffer{nullptr};
    std::unique_ptr<Buffer> uvBuffer{nullptr};
    std::unique_ptr<Buffer> indexBuffer{nullptr};
    std::unique_ptr<Buffer> materialsBuffer{nullptr};
    std::unique_ptr<Buffer> sceneDescBuffer{nullptr};

    std::vector<Buffer> transformBuffers{};

    std::vector<Accel> blases;
    Accel tlas;


    std::vector<RTLight> lights;
    std::vector<RTPrimitive> primitives;
    std::vector<RTMaterial> materials;

    std::unique_ptr<SgImage> storageImage;


    uint32_t width,height;
    std::shared_ptr<Camera> camera{nullptr};

    friend class RayTracer;
    
    
    RenderContext * renderContext;
    Device & device;
};
