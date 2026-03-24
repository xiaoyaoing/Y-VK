#include "RayTracingEditorRenderer.h"

#include "app/editor/EditorApplication.h"

RTRenderer& RayTracingEditorRendererBase::renderer() {
    return *mRenderer;
}

const RTRenderer& RayTracingEditorRendererBase::renderer() const {
    return *mRenderer;
}

void RayTracingEditorRendererBase::initialize(EditorApplication& editor) {
    mConfig = editor.getConfig();
    mRenderer = createRenderer();
    mRenderer->initialize(editor.getDevice(), mConfig);
}

void RayTracingEditorRendererBase::onActivated(EditorApplication&) {
    g_context->setFlipViewport(false);
}

void RayTracingEditorRendererBase::onSceneLoaded(EditorApplication& editor) {
    if (editor.getScene()) {
        editor.getScene()->addDirectionalLight(glm::vec3(0.0, -1.0, 0.3), glm::vec3(1.0f), 1.5f);
    }
    editor.finalizeSceneLoaded();
    renderer().onSceneLoaded(editor.getDevice(), *editor.getScene());
}

void RayTracingEditorRendererBase::render(EditorApplication& editor, RenderGraph& renderGraph) {
    renderer().render(renderGraph, editor.getCamera());
}

void RayTracingEditorRendererBase::updateGui(EditorApplication&) {
    renderer().updateGui();
}

std::string RayTracingEditorRendererBase::getHdrImageToSave() const {
    return renderer().getHdrImageToSave();
}

std::unique_ptr<RTRenderer> PathTracingEditorRenderer::createRenderer() const {
    return std::make_unique<PathTracingRTRenderer>();
}

std::unique_ptr<RTRenderer> DDGIEditorRenderer::createRenderer() const {
    return std::make_unique<DDGIRTRenderer>();
}

std::unique_ptr<RTRenderer> RestirDIEditorRenderer::createRenderer() const {
    return std::make_unique<RestirDIRTRenderer>();
}
