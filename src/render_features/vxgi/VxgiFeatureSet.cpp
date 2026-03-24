#include "VxgiFeatureSet.h"

#include "ClipmapCleaner.h"
#include "CopyAlphaPass.h"
#include "FinalLightingPass.h"
#include "LightInjectionPass.h"
#include "RenderPasses/GBufferPass.h"
#include "RenderPasses/ShadowMapPass.h"
#include "VoxelizationPass.h"
#include "Scene/Compoments/Camera.h"
#include "imgui.h"

void VxgiFeatureSet::initialize() {
    passes.emplace_back(std::make_unique<GBufferPass>());
    passes.emplace_back(std::make_unique<ShadowMapPass>());
    passes.emplace_back(std::make_unique<VoxelizationPass>());
    passes.emplace_back(std::make_unique<LightInjectionPass>());
    passes.emplace_back(std::make_unique<CopyAlphaPass>());
    passes.emplace_back(std::make_unique<FinalLightingPass>());

    ClipMapCleaner::init();

    for (auto& pass : passes) {
        pass->init();
    }

    mVisualizeVoxelPass.init();
}

void VxgiFeatureSet::render(RenderGraph& rg, Camera& camera) {
    VxgiContext::OnFrameBegin();
    updateClipRegions(camera);

    for (auto& pass : passes) {
        pass->render(rg);
    }

    drawVoxelVisualization(rg);
    rg.setCutUnUsedResources(false);
}

void VxgiFeatureSet::updateGui() {
    for (auto& pass : passes) {
        pass->updateGui();
    }

    ImGui::Checkbox("C1", &mVisualizeClipRegion[0]);
    ImGui::SameLine();
    ImGui::Checkbox("C2", &mVisualizeClipRegion[1]);
    ImGui::SameLine();
    ImGui::Checkbox("C3", &mVisualizeClipRegion[2]);
    ImGui::SameLine();
    ImGui::Checkbox("C4", &mVisualizeClipRegion[3]);
    ImGui::SameLine();
    ImGui::Checkbox("C5", &mVisualizeClipRegion[4]);
    ImGui::SameLine();
    ImGui::Checkbox("C6", &mVisualizeClipRegion[5]);
    ImGui::SameLine();
    ImGui::Checkbox("Radiance", &mVisualizeRadiance);

    VxgiContext::Gui();
}

void VxgiFeatureSet::drawVoxelVisualization(RenderGraph& renderGraph) {
    auto& regions = VxgiContext::getClipmapRegions();
    bool  clear   = true;
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        if (mVisualizeClipRegion[i]) {
            const std::string name = mVisualizeRadiance ? "radiance" : "opacity";
            mVisualizeVoxelPass.visualize3DClipmapGS(renderGraph, renderGraph.getBlackBoard().getHandle(name), regions[i], i, i > 0 ? &regions[i - 1] : nullptr, true, 3, clear);
            clear = false;
        }
    }
}

BBox VxgiFeatureSet::getBBox(const Camera& camera, uint32_t clipmapLevel) const {
    const float halfSize = 0.5f * mClipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));
    return {camera.getPosition() - halfSize, camera.getPosition() + halfSize};
}

void VxgiFeatureSet::updateClipRegions(Camera& camera) {
    if (VxgiContext::getBBoxes().empty()) {
        for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
            VxgiContext::getBBoxes().push_back(getBBox(camera, i));
        }
    }

    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        VxgiContext::getBBoxes()[i] = getBBox(camera, i);
    }
}
