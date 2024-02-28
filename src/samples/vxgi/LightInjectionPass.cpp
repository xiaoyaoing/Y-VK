#include "LightInjectionPass.h"

#include "Core/RenderContext.h"
#include "Core/View.h"
#include "Scene/Scene.h"
void LightInjectionPass::render(RenderGraph& rg) {
    auto& commandBuffer = g_context->getGraphicCommandBuffer();
    rg.addPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {
        auto radiance  = rg.importTexture("radiance", mLightInjectionImage.get());

        builder.writeTexture(radiance);   
        // builder.readTexture(normal);     

        RenderGraphPassDescriptor desc{};
        desc.textures = {radiance};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {}});
            builder.declare(desc); }, [&](RenderPassContext& context) {
            auto& view       = *g_manager->fetchPtr<View>("view");
            view.bindViewBuffer().bindViewShading();
            g_context->getPipelineState().setPipelineLayout(*mLightInjectionPipelineLayout);
            g_context->bindImage(2, mLightInjectionImage->getVkImageView());
            for(auto& primitive : view.getMVisiblePrimitives()) {
                g_context->bindPrimitiveGeom(commandBuffer,*primitive).bindPrimitiveShading(commandBuffer,*primitive);
                g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
            } });
}
void LightInjectionPass::init() {
    VkExtent3D imageResolution    = {VOXEL_RESOLUTION * 6,
                                     VOXEL_RESOLUTION * CLIP_MAP_LEVEL,
                                     VOXEL_RESOLUTION};
    mLightInjectionPipelineLayout = std::make_unique<PipelineLayout>(
        g_context->getDevice(), std::vector<std::string>{"vxgi/lightInjection.vert", "vxgi/lightInjection.frag"});
    mLightInjectionImage = std::make_unique<SgImage>(
        g_context->getDevice(), std::string("voxelRadianceImage"), imageResolution, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);
}