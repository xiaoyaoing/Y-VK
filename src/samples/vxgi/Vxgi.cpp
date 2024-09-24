//
// Created by pc on 2023/8/17.
//

#include "Vxgi.h"

#include "ClipmapCleaner.h"
#include "CopyAlphaPass.h"
#include "FinalLightingPass.h"
#include "RenderPasses/GBufferPass.h"
#include "LightInjectionPass.h"
#include "VoxelizationPass.h"
#include "../../framework/Common/VkCommon.h"
#include "Common/FIleUtils.h"
#include "Core/Shader/GlslCompiler.h"
#include "Core/Shader/Shader.h"
#include "RenderPasses/ShadowMapPass.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"

void VXGI::drawFrame(RenderGraph& rg) {

    updateClipRegions();

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();
    auto& blackBoard    = rg.getBlackBoard();

    //    if (frameCounter++ < 2) {
    for (auto& pass : passes) {
        pass->render(rg);
    }
    drawVoxelVisualization(rg);
    //  }

    rg.setCutUnUsedResources(false);
}
void VXGI::drawVoxelVisualization(RenderGraph& renderGraph) {
    auto& regions = VxgiContext::getClipmapRegions();
    bool  clear   = true;
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        if (m_visualizeClipRegion[i]) {
            std::string name = mVisualizeRadiance ? "radiance" : "opacity";
            mVisualizeVoxelPass.visualize3DClipmapGS(renderGraph, renderGraph.getBlackBoard().getHandle(name), regions[i], i, i > 0 ? &regions[i - 1] : nullptr, true, 3, clear);
            clear = false;
        }
    }
}

BBox VXGI::getBBox(uint32_t clipmapLevel) {
    float halfSize = 0.5f * m_clipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));
    return {camera->getPosition() - halfSize, camera->getPosition() + halfSize};
}

void VXGI::prepare() {
    Application::prepare();

    
   sceneLoadingConfig.sceneScale = glm::vec3(0.3f);
   // loadScene(FileUtils::getResourcePath("sponza/Sponza01.gltf"));
    loadScene("E:/code/moerengine-3dgs/asset/default/scenes/sponza/Sponza01.gltf");
    // loadScene("E:/code/vkframeworklearn2/resources/cornell-box/cornellBox.gltf");
    
    GlslCompiler::forceRecompile = false;

    auto light_pos   = glm::vec3(0.0f, 128.0f, -225.0f);
    auto light_color = glm::vec3(1.0, 1.0, 1.0);

    for (int i = -4; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            glm::vec3 pos = light_pos;
            pos.x += i * 400;
            pos.z += j * (225 + 140);
            pos.y = 8;

            for (int k = 0; k < 3; ++k) {
                pos.y = pos.y + (k * 100);

                light_color.x = static_cast<float>(rand()) / (RAND_MAX);
                light_color.y = static_cast<float>(rand()) / (RAND_MAX);
                light_color.z = static_cast<float>(rand()) / (RAND_MAX);

                LightProperties props;
                props.color     = light_color;
                props.intensity = 0.2f;
                props.position  = pos;

                //  scene->addLight(SgLight{.type = LIGHT_TYPE::Point, .lightProperties = props});
            }
        }
    }
    
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        VxgiContext::getBBoxes().push_back(getBBox(i));
    }

    passes.emplace_back(std::make_unique<GBufferPass>());
    // passes.emplace_back(std::make_unique<ShadowMapPass>());
    passes.emplace_back(std::make_unique<VoxelizationPass>());
    passes.emplace_back(std::make_unique<LightInjectionPass>());
    passes.emplace_back(std::make_unique<CopyAlphaPass>());
    passes.emplace_back(std::make_unique<FinalLightingPass>());
    
    g_manager->putPtr("scene", scene.get());
    g_manager->putPtr("view", view.get());
    g_manager->putPtr("camera", camera.get());

    ClipMapCleaner::init();

    for (auto& pass : passes) {
        pass->init();
    }

    mVisualizeVoxelPass.init();
}

VXGI::VXGI() : Application("VXGI", 1920, 1080) {
    // addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);//   addInstanceExtension(VK_EXT_debug_utils);
}

void VXGI::onUpdateGUI() {
    for (auto& pass : passes) {
        pass->updateGui();
    }
    ImGui::Checkbox("C1", &m_visualizeClipRegion[0]); 
    ImGui::SameLine();
    ImGui::Checkbox("C2", &m_visualizeClipRegion[1]);
    ImGui::SameLine();
    ImGui::Checkbox("C3", &m_visualizeClipRegion[2]);
    ImGui::SameLine();
    ImGui::Checkbox("C4", &m_visualizeClipRegion[3]);
    ImGui::SameLine();
    ImGui::Checkbox("C5", &m_visualizeClipRegion[4]);
    ImGui::SameLine();
    ImGui::Checkbox("C6", &m_visualizeClipRegion[5]);
    ImGui::SameLine();
    ImGui::Checkbox("Radiance", &mVisualizeRadiance);
}

void VXGI::updateClipRegions() {
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        VxgiContext::getBBoxes()[i] = getBBox(i);
    }
}
void VXGI::onSceneLoaded() {
    scene->addDirectionalLight({0, -0.95f, 0.3f}, glm::vec3(1.0f), 1.5f);
    Application::onSceneLoaded();
}

int main() {
    auto example = new VXGI();
    example->prepare();
    example->mainloop();
    return 0;
}