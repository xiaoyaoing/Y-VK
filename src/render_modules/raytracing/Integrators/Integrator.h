#pragma once
#include "../scene/RTScene.h"
#include "Core/RenderContext.h"
#include <Raytracing/commons.h>

class Integrator {
public:
    Integrator(Device& device);
    virtual ~Integrator();

    virtual void init();
    virtual void initScene(RTSceneEntry& entry);
    virtual void render(RenderGraph& renderGraph) = 0;
    virtual void updateGui();
    virtual void destroy();
    virtual void update();
    void initLightAreaDistribution(RenderGraph& graph);

    virtual void bindRaytracingResources(CommandBuffer& commandBuffer);
    virtual void onUpdateGUI() {}

    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

protected:
    RTSceneEntry* entry_;
    const Scene* mScene{nullptr};
    PipelineLayout* computePrimAreaLayout;
    std::shared_ptr<SgImage> storageImage;
    uint32_t width, height;
    std::shared_ptr<Camera> camera{nullptr};
    RenderContext* renderContext;
    Device& device;
};
