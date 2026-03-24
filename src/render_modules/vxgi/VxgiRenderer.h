#pragma once

#include "app/editor/Renderer.h"
#include "ClipmapRegion.h"
#include "ClipmapUpdatePolicy.h"
#include "VisualizeVoxelPass.h"
#include "VxgiCommon.h"

class PassBase;

class VxgiRenderer final : public EditorRenderer {
public:
    std::string_view getName() const override { return "VXGI"; }
    void initialize(EditorApplication& editor) override;
    void onActivated(EditorApplication& editor) override;
    void onSceneLoaded(EditorApplication& editor) override;
    void render(EditorApplication& editor, RenderGraph& renderGraph) override;
    void updateGui(EditorApplication& editor) override;

private:
    void drawVoxelVisualization(RenderGraph& renderGraph);
    void updateClipRegions(EditorApplication& editor);
    BBox getBBox(EditorApplication& editor, uint32_t clipmapLevel);

    std::vector<std::unique_ptr<PassBase>> passes{};
    std::unique_ptr<ClipmapUpdatePolicy> mClipmapUpdatePolicy{nullptr};
    bool mVisualizeClipRegion[6]{false, false, false, false, false, false};
    bool mVisualizeRadiance = true;
    bool injectLight = false;
    VisualizeVoxelPass mVisualizeVoxelPass;
    inline static uint32_t mClipRegionBBoxExtentL0 = 16;
    bool mInitialized = false;
};
