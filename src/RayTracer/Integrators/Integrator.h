#pragma once
#include "../Utils/RTSceneUtil.h"
#include "Core/RenderContext.h"
#include <Raytracing/commons.h>


class Integrator {
public:
    Integrator(Device& device);
    virtual ~Integrator();

    virtual void init();
    virtual void initScene(RTSceneEntry & entry);
    virtual void render(RenderGraph& renderGraph) = 0;
    virtual void updateGui();
    virtual void destroy();
    virtual void update();
    void initLightAreaDistribution(RenderGraph& graph);

    virtual void bindRaytracingResources(CommandBuffer& commandBuffer);


    virtual void onUpdateGUI(){};

protected:

    // Buffer* vertexBuffer{nullptr};
    // Buffer* normalBuffer{nullptr};
    // Buffer* uvBuffer{nullptr};
    // Buffer* indexBuffer{nullptr};
    //
    // std::shared_ptr<Buffer> materialsBuffer{nullptr};
    // std::shared_ptr<Buffer> primitiveMeshBuffer{nullptr};
    // std::shared_ptr<Buffer> rtLightBuffer{nullptr};
    // std::vector<Buffer>     transformBuffers{};
    //

    //
    // const std::vector<Accel> * blases;
    // const Accel *             tlas;
    //
    // const std::vector<RTLight> *    lights;
    // const std::vector<RTPrimitive> * primitives;
    // const std::vector<RTMaterial> * materials;

    RTSceneEntry * entry_;

    const Scene* mScene{nullptr};

    // std::unordered_map<uint32_t, std::shared_ptr<Buffer>> primAreaBuffers{};
    // std::unordered_map<uint32_t, std::shared_ptr<Buffer>> primAreaDistributionBuffers{};
    PipelineLayout *                   computePrimAreaLayout;

    std::shared_ptr<SgImage> storageImage;

    uint32_t                width, height;
    std::shared_ptr<Camera> camera{nullptr};

    friend class RayTracer;

    RenderContext* renderContext;
    Device&        device;
};
