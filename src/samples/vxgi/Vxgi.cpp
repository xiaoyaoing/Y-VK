//
// Created by pc on 2023/8/17.
//

#include "Vxgi.h"

#include "FinalLightingPass.h"
#include "GBufferPass.h"
#include "LightInjectionPass.h"
#include "VoxelizationPass.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Core/Shader/Shader.h"
#include "Scene/SceneLoader/gltfloader.h"

void Example::drawFrame(RenderGraph& rg) {

    updateClipRegions();

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();
    auto& blackBoard    = rg.getBlackBoard();

    for (auto& pass : passes)
        pass->render(rg);
    gui->addGuiPass(rg);

    rg.execute(commandBuffer);
}

BBox Example::getBBox(uint32_t clipmapLevel) {
    float halfSize = 0.5f * m_clipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));
    return {camera->position - halfSize, camera->position + halfSize};
}

void Example::prepare() {
    Application::prepare();

    scene = GltfLoading::LoadSceneFromGLTFFile(
        *device, FileUtils::getResourcePath("sponza/Sponza01.gltf"));

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());

    for (uint32_t i = 0; i < CLIP_MAP_LEVEL; i++) {
        mBBoxes[i] = getBBox(i);
    }

    VoxelizationPass pass;
    VxgiPassBase*    base = &pass;
    passes.emplace_back(std::make_unique<VoxelizationPass>());
    //passes.emplace_back(std::make_unique<LightInjectionPass>());
    passes.emplace_back(std::make_unique<FinalLightingPass>());

    g_manager->putPtr("scene", scene.get());
    g_manager->putPtr("camera", camera.get());
    g_manager->putPtr("bboxes", &mBBoxes);
}

Example::Example() : Application("Drawing Triangle", 1024, 1024) {
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
}

void Example::onUpdateGUI() {
}

void Example::updateClipRegions() {
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL; i++) {
        mBBoxes[i] = getBBox(i);
    }
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}