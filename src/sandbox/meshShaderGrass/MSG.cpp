//
// Created by pc on 2024/1/24.
//

#include "MSG.h"



#define VK_KHR_get_physical_device_properties2 1

#include "Common/VkCommon.h"
void MeshShaderGrass::drawFrame(RenderGraph& rg) {
    grassPass->render(rg);
}
MeshShaderGrass::MeshShaderGrass() : Application("MGS", 1920, 1080) {
    addDeviceExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
}
void MeshShaderGrass::prepare() {
    Application::prepare();
    scene = loadDefaultTriangleScene(*device);
    initView();
    grassPass = std::make_unique<GrassPass>();
}

void MeshShaderGrass::onUpdateGUI() {
    Application::onUpdateGUI();
    grassPass->updateGui();
}

MAIN(MeshShaderGrass)