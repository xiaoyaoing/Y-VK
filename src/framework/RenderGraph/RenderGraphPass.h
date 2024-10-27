#pragma once

#include "Enum.h"
#include "RenderGraphId.h"
#include "Core/PipelineState.h"
#include "Core/Vulkan.h"
#include "Common/BitMaskEnums.h"

class RenderTarget;
class CommandBuffer;
class RenderPass;
class Accel;
class PipelineLayout;
class RenderGraph;


template<>
struct EnableBitMaskOperators<RenderPassType> : std::true_type {};

struct GraphicPassSettings {
    RenderPassType type = RenderPassType::GRAPHICS;
};

struct ComputePassSettings {
    RenderPassType type = RenderPassType::COMPUTE;
    PipelineLayout *       pipelineLayout{nullptr};
};

struct RaytracingPassSettings {
    RTPipelineSettings     rTPipelineSettings;
    RenderPassType type = RenderPassType::RAYTRACING;
    Accel*                 accel{nullptr};
    PipelineLayout*        pipelineLayout{nullptr};
    ShaderPipelineKey shaderPaths;

    std::vector<RenderGraphHandle> inBuffer;
    std::vector<RenderGraphHandle> outBuffer;
    std::vector<RenderGraphHandle> inTextures;
    std::vector<RenderGraphHandle> outTextures;
};

struct RenderPassContext {
    CommandBuffer& commandBuffer;
    RenderGraph&   renderGraph;
};

using GraphicsExecute   = std::function<void(RenderPassContext& context)>;
using ComputeExecute    = std::function<void(RenderPassContext& context)>;
using RaytracingExecute = std::function<void(RenderPassContext& context)>;

class RenderGraphPassBase {
public:
    virtual void execute(RenderPassContext& context) = 0;

    virtual ~RenderGraphPassBase() = default;
};

template<typename Data, typename Execute>
class RenderGraphPass : public RenderGraphPassBase {
    Data    data;
    Execute mExecute;

public:
    explicit RenderGraphPass(Execute&& execute) noexcept
        : mExecute(std::move(execute)) {
    }

    void execute(RenderPassContext& context) override {
        mExecute(context);
    }

    Data& getData() { return data; }
};

using GraphicRenderGraphPass    = RenderGraphPass<GraphicPassSettings, GraphicsExecute>;
using ComputeRenderGraphPass    = RenderGraphPass<ComputePassSettings, ComputeExecute>;
using RaytracingRenderGraphPass = RenderGraphPass<RaytracingPassSettings, RaytracingExecute>;