//
// Created by pc on 2024/1/24.
//

#include "TinyNanite.h"

#define VK_KHR_get_physical_device_properties2 1

#include "Common/VkCommon.h"
void TinyNanite::drawFrame(RenderGraph& rg) {
    grassPass->render(rg);
}
TinyNanite::TinyNanite() : Application("MGS", 1920, 1080) {
    addDeviceExtension(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
}
void TinyNanite::prepare() {
    Application::prepare();
    scene = loadDefaultTriangleScene(*device);
    initView();
    grassPass = std::make_unique<GrassPass>();
}

void TinyNanite::onUpdateGUI() {
    Application::onUpdateGUI();
    grassPass->updateGui();
}

MAIN(TinyNanite)