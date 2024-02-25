#include "LightInjectionPass.h"

#include "Core/RenderContext.h"
#include "Core/View.h"
#include "Scene/Scene.h"
void LightInjectionPass::render(RenderGraph& rg) {
    auto& blackboard = rg.getBlackBoard();
    auto& view       = *g_manager->fetchPtr<View>("view");
    auto& commandBuffer = g_context->getGraphicCommandBuffer();
    rg.addPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {
     //   auto opacity = blackboard.getHandle("opacity");
       // auto normal = rg.getBlackBoard().getHandle("normal");
       // auto depth = rg.getBlackBoard().getHandle("depth");
        // auto albedo = rg.getBlackBoard().getHandle("albedo");
        auto voxelRadiance = rg.createTexture("voxelRadiance", {.extent =  {},.useage = TextureUsage::COLOR_ATTACHMENT});

        // builder.readTexture(normal);     

        RenderGraphPassDescriptor desc{};
        desc.textures = {voxelRadiance};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {voxelRadiance}});

        ("Light Injection Pass",desc); }, [&](RenderPassContext& context) {
            view.bindViewBuffer().bindViewShading();
            g_context->getPipelineState().setPipelineLayout(*mLightInjectionPipelineLayout);
            g_context->bindImage(0, mLightInjectionImage->getVkImageView());
            mScene->IteratePrimitives([&commandBuffer](const Primitive & primitive) {
                g_context->bindPrimitiveGeom(commandBuffer,primitive).bindPrimitiveShading(commandBuffer,primitive);
                g_context->flushAndDrawIndexed(commandBuffer, primitive.indexCount, 1, 0, 0, 0);
                                                                                                      }); });
}
void LightInjectionPass::init() {
}