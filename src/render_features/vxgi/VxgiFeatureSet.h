#pragma once

#include "VisualizeVoxelPass.h"
#include "VxgiCommon.h"

class Camera;
class PassBase;
class RenderGraph;

class VxgiFeatureSet {
public:
    void initialize();
    void render(RenderGraph& rg, Camera& camera);
    void updateGui();

private:
    void drawVoxelVisualization(RenderGraph& renderGraph);
    void updateClipRegions(Camera& camera);
    BBox getBBox(const Camera& camera, uint32_t clipmapLevel) const;

    std::vector<std::unique_ptr<PassBase>> passes;
    bool                                   mVisualizeClipRegion[6]{false, false, false, false, false, false};
    bool                                   mVisualizeRadiance = true;
    VisualizeVoxelPass                     mVisualizeVoxelPass;
    inline static uint32_t                 mClipRegionBBoxExtentL0 = 16;
};
