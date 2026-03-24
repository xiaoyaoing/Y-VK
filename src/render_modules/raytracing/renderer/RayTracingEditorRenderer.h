#pragma once

#include "app/editor/Renderer.h"
#include "RayTracingRenderer.h"

class RayTracingEditorRendererBase : public EditorRenderer {
public:
    void initialize(EditorApplication& editor) override;
    void onActivated(EditorApplication& editor) override;
    void onSceneLoaded(EditorApplication& editor) override;
    void render(EditorApplication& editor, RenderGraph& renderGraph) override;
    void updateGui(EditorApplication& editor) override;
    std::string getHdrImageToSave() const override;

protected:
    virtual std::unique_ptr<RTRenderer> createRenderer() const = 0;
    RTRenderer& renderer();
    const RTRenderer& renderer() const;

private:
    std::unique_ptr<RTRenderer> mRenderer;
    RTConfing mConfig;
};

class PathTracingEditorRenderer final : public RayTracingEditorRendererBase {
public:
    std::string_view getName() const override { return "Path Tracing"; }
protected:
    std::unique_ptr<RTRenderer> createRenderer() const override;
};

class DDGIEditorRenderer final : public RayTracingEditorRendererBase {
public:
    std::string_view getName() const override { return "DDGI"; }
protected:
    std::unique_ptr<RTRenderer> createRenderer() const override;
};

class RestirDIEditorRenderer final : public RayTracingEditorRendererBase {
public:
    std::string_view getName() const override { return "ReSTIR DI"; }
protected:
    std::unique_ptr<RTRenderer> createRenderer() const override;
};
