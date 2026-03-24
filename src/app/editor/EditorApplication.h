#pragma once

#include "App/Application.h"
#include "Renderer.h"
#include "RendererRegistry.h"

#include <memory>
#include <string>
#include <vector>

class EditorApplication : public Application {
public:
    EditorApplication();

    void prepare() override;
    void onSceneLoaded() override;
    void onUpdateGUI() override;
    void drawFrame(RenderGraph& renderGraph) override;
    std::string getLdrImageToSave() override;
    std::string getHdrImageToSave() override;

    Device& getDevice();
    RenderContext& getRenderContext();
    Scene* getScene();
    Camera& getCamera();
    std::shared_ptr<Camera> getCameraHandle();
    View* getView();
    Gui* getGui();
    RTConfing& getConfig();
    const RTConfing& getConfig() const;

    void requireDeviceExtension(const char* extension, bool optional = true) { addDeviceExtension(extension, optional); }
    void requireInstanceExtension(const char* extension, bool optional = true) { addInstanceExtension(extension, optional); }

    void finalizeSceneLoaded();

private:
    EditorRenderer& currentRenderer();
    const EditorRenderer& currentRenderer() const;
    EditorRenderer& ensureRendererInitialized(int index);
    void initializeRendererMetadata();
    void selectRenderer(int index);
    std::string resolveDefaultScenePath() const;
    void setupEditorSceneLoading();

    std::vector<std::unique_ptr<EditorRenderer>> mRenderers;
    std::vector<std::string> mRendererNames;
    int mCurrentRendererIndex = 0;
    std::string mCurrentScenePath;
};
