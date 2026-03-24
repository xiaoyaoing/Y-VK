#include "EditorApplication.h"

#include "Common/Config.h"
#include "Common/FIleUtils.h"
#include "Common/VkCommon.h"
#include "imgui.h"

namespace {
std::string getCornellBoxPath(const RTConfing& config) {
    const std::string candidates[] = {
        FileUtils::getResourcePath("cornellBox.gltf"),
        FileUtils::getResourcePath("/cornellBox.gltf"),
        config.getScenePath(),
    };

    for (const auto& candidate : candidates) {
        if (!candidate.empty() && FileUtils::fileExists(candidate)) {
            return candidate;
        }
    }

    return config.getScenePath();
}
} // namespace

EditorApplication::EditorApplication()
    : Application(
          "YVK Editor",
          RTConfing(FileUtils::getResourcePath("render.json")).getWindowWidth(),
          RTConfing(FileUtils::getResourcePath("render.json")).getWindowHeight(),
          RTConfing(FileUtils::getResourcePath("render.json"))) {
    for (const auto& descriptor : getRendererRegistry()) {
        descriptor.configureEditor(*this);
    }
}

Device& EditorApplication::getDevice() { return *device; }
RenderContext& EditorApplication::getRenderContext() { return *renderContext; }
Scene* EditorApplication::getScene() { return scene.get(); }
Camera& EditorApplication::getCamera() { return *camera; }
std::shared_ptr<Camera> EditorApplication::getCameraHandle() { return camera; }
View* EditorApplication::getView() { return view.get(); }
Gui* EditorApplication::getGui() { return gui.get(); }
RTConfing& EditorApplication::getConfig() { return config; }
const RTConfing& EditorApplication::getConfig() const { return config; }

EditorRenderer& EditorApplication::ensureRendererInitialized(int index) {
    if (!mRenderers[index]) {
        mRenderers[index] = getRendererRegistry()[index].create();
        mRenderers[index]->initialize(*this);
        if (scene) {
            mRenderers[index]->onSceneLoaded(*this);
        }
    }
    return *mRenderers[index];
}

EditorRenderer& EditorApplication::currentRenderer() {
    return ensureRendererInitialized(mCurrentRendererIndex);
}

const EditorRenderer& EditorApplication::currentRenderer() const {
    return *mRenderers[mCurrentRendererIndex];
}

void EditorApplication::initializeRendererMetadata() {
    mRenderers.clear();
    mRenderers.resize(getRendererRegistry().size());
    mRendererNames.clear();
    for (const auto& descriptor : getRendererRegistry()) {
        mRendererNames.emplace_back(std::string(descriptor.name));
    }
}

void EditorApplication::setupEditorSceneLoading() {
    sceneLoadingConfig = {};
    sceneLoadingConfig.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME};
    sceneLoadingConfig.enableMergeDrawCalls = false;
    sceneLoadingConfig.indexType = VK_INDEX_TYPE_UINT32;
    sceneLoadingConfig.bufferAddressAble = true;
    sceneLoadingConfig.bufferForAccel = true;
    sceneLoadingConfig.bufferForStorage = true;
    sceneLoadingConfig.loadLight = true;
}

std::string EditorApplication::resolveDefaultScenePath() const {
    return getCornellBoxPath(config);
}

void EditorApplication::selectRenderer(int index) {
    if (index < 0 || index >= static_cast<int>(mRenderers.size()) || index == mCurrentRendererIndex) {
        return;
    }

    mCurrentRendererIndex = index;
    ensureRendererInitialized(mCurrentRendererIndex).onActivated(*this);
}

void EditorApplication::prepare() {
    Application::prepare();
    initializeRendererMetadata();
    setupEditorSceneLoading();
    mCurrentScenePath = resolveDefaultScenePath();
    loadScene(mCurrentScenePath);
    ensureRendererInitialized(mCurrentRendererIndex).onActivated(*this);
}

void EditorApplication::finalizeSceneLoaded() {
    Application::onSceneLoaded();
}

void EditorApplication::onSceneLoaded() {
    currentRenderer().onSceneLoaded(*this);
}

void EditorApplication::drawFrame(RenderGraph& renderGraph) {
    currentRenderer().render(*this, renderGraph);
}

void EditorApplication::onUpdateGUI() {
    int selectedRenderer = mCurrentRendererIndex;
    std::vector<const char*> names;
    names.reserve(mRendererNames.size());
    for (const auto& name : mRendererNames) {
        names.push_back(name.c_str());
    }

    if (!names.empty() && ImGui::Combo("Renderer", &selectedRenderer, names.data(), static_cast<int>(names.size()))) {
        selectRenderer(selectedRenderer);
    }

    currentRenderer().updateGui(*this);
}

std::string EditorApplication::getLdrImageToSave() {
    return currentRenderer().getLdrImageToSave();
}

std::string EditorApplication::getHdrImageToSave() {
    return currentRenderer().getHdrImageToSave();
}
