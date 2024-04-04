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
#include "Scene/SceneLoader/gltfloader.h"

void Example::drawFrame(RenderGraph& rg) {

    updateClipRegions();

    auto& commandBuffer = renderContext->getGraphicCommandBuffer();
    auto& blackBoard    = rg.getBlackBoard();

    for (auto& pass : passes)
        pass->render(rg);
    drawVoxelVisualization(rg);
    gui->addGuiPass(rg);

    rg.setCutUnUsedResources(false);
    rg.execute(commandBuffer);
}
void Example::drawVoxelVisualization(RenderGraph& renderGraph) {
    auto& regions = *g_manager->fetchPtr<std::vector<ClipmapRegion>>("clipmap_regions");
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        if (m_visualizeClipRegion[i]) {
            mVisualizeVoxelPass.visualize3DClipmapGS(renderGraph, renderGraph.getBlackBoard().getHandle("radiance"), regions[i], i, i > 0 ? &regions[i - 1] : nullptr, false, 3);
        }
    }
}

BBox Example::getBBox(uint32_t clipmapLevel) {
    float halfSize = 0.5f * m_clipRegionBBoxExtentL0 * std::exp2f(float(clipmapLevel));
    return {camera->position - halfSize, camera->position + halfSize};
}

void Example::prepare() {
    Application::prepare();

    //Shader shader(*device,"E:/code/moerengine2/shaders/3dgs_splatting/preprocess.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    Shader shader(*device, "E:/code/moerengine2/shaders/3dgs_splatting/tile_boundary.spv", VK_SHADER_STAGE_COMPUTE_BIT);

    scene = GltfLoading::LoadSceneFromGLTFFile(
        *device, FileUtils::getResourcePath("sponza/Sponza01.gltf"), {.sceneTransform = glm::scale(glm::mat4(1.f), glm::vec3(0.008f))});

    // scene = GltfLoading::LoadSceneFromGLTFFile(
    //     *device, "E:/code/vk_vxgi/VFS/Scene/Sponza/Sponza.gltf", {.bufferRate = BufferRate::PER_SCENE});

    // scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("cornell-box/cornellBox.gltf"));

    GlslCompiler::forceRecompile = true;

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
    scene->addDirectionalLight({0, -0.95f, 0.3f}, glm::vec3(1.0f), 1.5f);
    // scene->addDirectionalLight({0.0f, 1.0f, 0.0f}, glm::vec3(1.0f), 0.5f);
    // scene->addDirectionalLight({1.0f, 0, 0}, glm::vec3(1.0f), 0.5f);
    // scene->addDirectionalLight({-1.0f, 0, 0}, glm::vec3(1.0f), 0.5f);

    // camera        = scene->getCameras()[0];
    // camera->flipY = true;
    // camera->setTranslation(glm::vec3(-494.f, -116.f, 99.f));
    // camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    // camera->setRotation(glm::vec3(0.0f, -90.0f, 0.0f));
    // camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 1.f, 4000.f);

    camera        = scene->getCameras()[0];
    camera->flipY = true;
    camera->setTranslation(glm::vec3(-2.69, 6.69, 1.5f));
    camera->setRotation(glm::vec3(-4.f, -269, 0.0f));
    camera->setPerspective(60.0f, (float)mWidth / (float)mHeight, 0.1f, 4000.f);
    camera->setMoveSpeed(0.05f);

    view = std::make_unique<View>(*device);
    view->setScene(scene.get());
    view->setCamera(camera.get());

    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        mBBoxes.push_back(getBBox(i));
    }

    passes.emplace_back(std::make_unique<GBufferPass>());
    passes.emplace_back(std::make_unique<VoxelizationPass>());
    passes.emplace_back(std::make_unique<LightInjectionPass>());
    passes.emplace_back(std::make_unique<CopyAlphaPass>());
    passes.emplace_back(std::make_unique<FinalLightingPass>());

    g_manager = new RenderPtrManangr();

    g_manager->putPtr("scene", scene.get());
    g_manager->putPtr("view", view.get());
    g_manager->putPtr("camera", camera.get());
    g_manager->putPtr("bboxes", &mBBoxes);
    g_manager->putPtr("clipmap_update_policy", &mClipmapUpdatePolicy);

    ClipMapCleaner::init();

    for (auto& pass : passes) {
        pass->init();
    }

    mVisualizeVoxelPass.init();
}

Example::Example() : Application("Drawing Triangle", 1024, 1024) {
    // addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);//   addInstanceExtension(VK_EXT_debug_utils);
}

void Example::onUpdateGUI() {
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
    ImGui::Checkbox("inject", &injectLight);
}

void Example::updateClipRegions() {
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        mBBoxes[i] = getBBox(i);
    }
}

int main() {
    auto example = new Example();
    example->prepare();
    example->mainloop();
    return 0;
}