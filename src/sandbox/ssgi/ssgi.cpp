//
// Created by pc on 2024/1/24.
//

#include "Triangle.h"

#define VK_KHR_get_physical_device_properties2 1

#include "Common/VkCommon.h"
void Example::drawFrame(RenderGraph& rg) {
    auto& commandBuffer = renderContext->getGraphicCommandBuffer();
    rg.addPass(
        "TrianglePass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            const auto output =  rg.getBlackBoard().getHandle(SWAPCHAIN_IMAGE_NAME);
            builder.declare(RenderGraphPassDescriptor({output},{.outputAttachments =  {output}})); }, [&](RenderPassContext& context) {
            view->bindViewBuffer();
            renderContext->getPipelineState().setPipelineLayout(*layout).setDepthStencilState({.depthTestEnable = false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
                
            if(this->rasterizationConservative) { 
                renderContext->getPipelineState().setRasterizationState({.cullMode = VK_CULL_MODE_NONE,.pNext =  &conservativeRasterizationStateCreateInfo});
            }
            for(const auto & prim : view->getMVisiblePrimitives()) {
                renderContext->bindPrimitive(commandBuffer,*prim).flushAndDrawIndexed(commandBuffer,prim->indexCount);
            } });
    gui->addGuiPass(rg);

    rg.compile();

    rg.execute(commandBuffer);
}
Example::Example() : Application("Triangle", 1920, 1080) {
    addDeviceExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME);
    addInstanceExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    rasterizationConservative = false;
}
void Example::prepare() {
    Application::prepare();

    std::vector<Shader> shaders{
        Shader(*device, FileUtils::getShaderPath("triangle/triangle.vert")),
        Shader(*device, FileUtils::getShaderPath("triangle/triangle.frag"))};
    this->layout = std::make_unique<PipelineLayout>(*device, shaders);

    scene = loadDefaultTriangleScene(*device);

    conservativeRasterizationProperties      = vkCommon::conservativeRasterizationProperties(device->getPhysicalDevice());
    conservativeRasterizationStateCreateInfo = vkCommon::rasterizationConservativeStateCreateInfo(conservativeRasterizationProperties.maxExtraPrimitiveOverestimationSize);

    // camera = scene->getCameras()[0];
    // camera->setPerspective(60.0f, float(mWidth)/float(mHeight), 0.1f, 256.0f);
    initView();
    camera->setTranslation({0, 0, -2});
}

void Example::onUpdateGUI() {
    ImGui::Checkbox("Conservative Rasterization", &rasterizationConservative);
}

EXAMPLE_MAIN