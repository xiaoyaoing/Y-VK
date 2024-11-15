//
// Created by pc on 2023/8/17.
//
#pragma once

#include "ClipmapRegion.h"
#include "ClipmapUpdatePolicy.h"
#include "VisualizeVoxelPass.h"
#include "VxgiCommon.h"
#include "App/Application.h"

class PassBase;
class VXGI : public Application {
public:
    void prepare() override;

    VXGI();

protected:
    void onUpdateGUI() override;
    void drawFrame(RenderGraph& renderGraph) override;
    void drawVoxelVisualization(RenderGraph& renderGraph);
    void updateClipRegions();
    void onSceneLoaded() override;

    BBox getBBox(uint32_t clipmapLevel);

    std::vector<std::unique_ptr<PassBase>> passes{};
    std::vector<BBox>                      mBBoxes{};
    std::unique_ptr<ClipmapUpdatePolicy>   mClipmapUpdatePolicy{nullptr};
    bool                                   m_visualizeClipRegion[6]{false, false, false, false, false, false};
    bool                                   mVisualizeRadiance = true;
    bool                                   injectLight        = false;
    VisualizeVoxelPass                     mVisualizeVoxelPass;
    inline static uint32_t                 m_clipRegionBBoxExtentL0 = 16;
};