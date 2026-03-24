#include "VxgiRenderer.h"

#include "app/editor/EditorApplication.h"

#include "ClipmapCleaner.h"
#include "CopyAlphaPass.h"
#include "Core/Shader/GlslCompiler.h"
#include "FinalLightingPass.h"
#include "LightInjectionPass.h"
#include "RenderPasses/GBufferPass.h"
#include "RenderPasses/ShadowMapPass.h"
#include "VoxelizationPass.h"
#include "imgui.h"

BBox VxgiRenderer::getBBox(EditorApplication& editor, uint32_t clipmapLevel) {
    float halfSize = 0.5f * mClipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));
    return {editor.getCamera().getPosition() - halfSize, editor.getCamera().getPosition() + halfSize};
}

void VxgiRenderer::updateClipRegions(EditorApplication& editor) {
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        VxgiContext::getBBoxes()[i] = getBBox(editor, i);
    }
}

void VxgiRenderer::initialize(EditorApplication& editor) {
    if (mInitialized) {
        return;
    }

    GlslCompiler::forceRecompile = true;

    if (VxgiContext::getBBoxes().empty()) {
        for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
            VxgiContext::getBBoxes().push_back(BBox{});
        }
    }

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
    mInitialized = true;
}

void VxgiRenderer::onActivated(EditorApplication&) {
    g_context->setFlipViewport(false);
}

void VxgiRenderer::onSceneLoaded(EditorApplication& editor) {
    if (editor.getScene()) {
        editor.getScene()->addDirectionalLight(glm::vec3(0.0, -1.0, 0.3), glm::vec3(1.0f), 1.5f, vec3(0.0f, 20, -5.f));
    }
    editor.finalizeSceneLoaded();
    g_manager->putPtr("scene", editor.getScene());
    g_manager->putPtr("view", editor.getView());
    g_manager->putPtr("camera", &editor.getCamera());
    updateClipRegions(editor);
}

void VxgiRenderer::drawVoxelVisualization(RenderGraph& renderGraph) {
    auto& regions = VxgiContext::getClipmapRegions();
    bool clear = true;
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        if (mVisualizeClipRegion[i]) {
            std::string name = mVisualizeRadiance ? "radiance" : "opacity";
            mVisualizeVoxelPass.visualize3DClipmapGS(renderGraph, renderGraph.getBlackBoard().getHandle(name), regions[i], i, i > 0 ? &regions[i - 1] : nullptr, true, 3, clear);
            clear = false;
        }
    }
}

void VxgiRenderer::render(EditorApplication& editor, RenderGraph& rg) {
    VxgiContext::OnFrameBegin();
    updateClipRegions(editor);
    for (auto& pass : passes) {
        pass->render(rg);
    }
    drawVoxelVisualization(rg);
    rg.setCutUnUsedResources(false);
}

void VxgiRenderer::updateGui(EditorApplication&) {
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
