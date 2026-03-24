#pragma once

#include "app/editor/Renderer.h"
#include "Rendering/IBL.h"
#include "RenderPasses/RenderPassBase.h"

class PbrRenderer final : public EditorRenderer {
public:
    std::string_view getName() const override { return "PBR"; }
    void initialize(EditorApplication& editor) override;
    void onActivated(EditorApplication& editor) override;
    void onSceneLoaded(EditorApplication& editor) override;
    void render(EditorApplication& editor, RenderGraph& renderGraph) override;
    void updateGui(EditorApplication& editor) override;

private:
    std::vector<std::unique_ptr<PassBase>> mRenderPasses;
    std::vector<std::unique_ptr<PassBase>> mForwardRenderPasses;
    std::unique_ptr<IBL> ibl;
    std::unique_ptr<Primitive> cube;
    std::unique_ptr<Texture> environmentCube{nullptr};
    std::unique_ptr<Texture> environmentCubeAsync{nullptr};
    float exposure = 4.5f;
    float gamma = 2.2f;
    bool mInitialized = false;
};
