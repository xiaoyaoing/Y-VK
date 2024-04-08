#pragma once
#include "Core/PipelineLayout.h"
#include "RenderGraph/RenderGraph.h"

struct ClipmapRegion;

class ClipMapCleaner {
public:
    static void clearClipMapRegions(RenderGraph& rg, const ClipmapRegion& clipRegion, RenderGraphHandle imageToClear, uint32_t clipLevel);
    static void init();

protected:
    ClipMapCleaner() = default;
    static inline std::unique_ptr<PipelineLayout> mPipelineLayout{nullptr};
    struct ImageCleaningDesc;
};
