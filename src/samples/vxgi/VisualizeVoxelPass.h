#pragma once
#include "ClipmapRegion.h"
#include "VxgiCommon.h"
#include "Core/Buffer.h"
#include "Core/PipelineLayout.h"
#include "RenderGraph/RenderGraphId.h"
#include "RenderPasses/RenderPassBase.h"
#include "Scene/Compoments/RenderPrimitive.h"

class VisualizeVoxelPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void visualize3DClipmapGS(RenderGraph& rg, RenderGraphHandle texture, const ClipmapRegion& region, uint32_t clipmapLevel, ClipmapRegion* prevRegion, bool hasMultipleFaces, int numColorComponents, bool clearDepth = true);
    void init() override;

protected:
    std::unique_ptr<PipelineLayout> mPipelineLayout;
    std::vector<Buffer>             mUniformBuffers;
    std::unique_ptr<Primitive>      mVoxelPrimitive;
};
