#pragma once
#include "Core/RenderContext.h"
#include "shaders/Raytracing/commons.h"


// struct RTModel
// {
//     std::vector<glm::vec3> position;
//     
// };






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

    virtual void bindRaytracingResources(CommandBuffer & commandBuffer);
    
    Accel createAccel(VkAccelerationStructureCreateInfoKHR& accel);
protected:
    SceneUbo sceneUbo;
    
    std::unique_ptr<Buffer> vertexBuffer{nullptr};
    std::unique_ptr<Buffer> normalBuffer{nullptr};
    std::unique_ptr<Buffer> uvBuffer{nullptr};
    std::unique_ptr<Buffer> indexBuffer{nullptr};
    
    std::unique_ptr<Buffer> materialsBuffer{nullptr};
    std::unique_ptr<Buffer> primitiveMeshBuffer{nullptr};
    std::unique_ptr<Buffer> rtLightBuffer{nullptr};
    std::vector<Buffer> transformBuffers{};
    
    std::unique_ptr<Buffer> sceneDescBuffer{nullptr};
    std::unique_ptr<Buffer> sceneUboBuffer{nullptr};
    
    std::vector<Accel> blases;
    Accel tlas;
    
    std::vector<RTLight> lights;
    std::vector<RTPrimitive> primitives;
    std::vector<RTMaterial> materials;
    std::vector<Texture *> textures;

    std::unique_ptr<SgImage> storageImage;


    uint32_t width,height;
    std::shared_ptr<Camera> camera{nullptr};


    friend class RayTracer;
    
    
    RenderContext * renderContext;
    Device & device;
};
