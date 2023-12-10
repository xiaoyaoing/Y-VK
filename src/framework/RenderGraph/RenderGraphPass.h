#pragma once

#include "RenderGraphId.h"
#include "Core/PipelineState.h"
#include "Core/Vulkan.h"

class RenderTarget;
class CommandBuffer;
class RenderPass;
class Accel;
class PipelineLayout;


enum RENDER_GRAPH_PASS_TYPE 
{
    GRAPHICS,COMPUTE,RAYTRACING
};


struct GraphicPassSettings
{
    RENDER_GRAPH_PASS_TYPE type = GRAPHICS;
};

struct ComputePassSettings
{
    RENDER_GRAPH_PASS_TYPE type = COMPUTE;

};

struct RaytracingPassSettings 
{
    RTPipelineSettings rTPipelineSettings;
    RENDER_GRAPH_PASS_TYPE  type = RAYTRACING;
    Accel * accel{nullptr};
    PipelineLayout * pipelineLayout{nullptr};
    
    std::vector<RenderGraphHandle> inBuffer;
    std::vector<RenderGraphHandle> outBuffer;
    std::vector<RenderGraphHandle> inTextures;
    std::vector<RenderGraphHandle> outTextures;
};

struct RenderPassContext {
    CommandBuffer &commandBuffer;
};

using GraphicsExecute = std::function<void(RenderPassContext & context)>;
using ComputeExecute = std::function<void(RenderPassContext & context)>;
using RaytracingExecute = std::function<void(RenderPassContext & context)>;




class RenderGraphPassBase {
public:
    virtual void execute(RenderPassContext &context) = 0;

    virtual ~RenderGraphPassBase() = default;
};

template<typename Data, typename Execute>
class RenderGraphPass : public RenderGraphPassBase {
    Data data;
    Execute mExecute;

public:
    
    
    explicit RenderGraphPass(Execute &&execute) noexcept
            : mExecute(std::move(execute)) {
    }

    void execute(RenderPassContext &context) override {
        mExecute(context);
    }

    Data &getData() { return data; }
};

using GraphicRenderGraphPass = RenderGraphPass<GraphicPassSettings,GraphicsExecute>;
using ComputeRenderGraphPass = RenderGraphPass<ComputePassSettings,ComputeExecute>;
using RaytracingRenderGraphPass  = RenderGraphPass<RaytracingPassSettings,RaytracingExecute>;
