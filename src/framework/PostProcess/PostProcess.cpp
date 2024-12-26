#include "PostProcess.h"

#include "imgui.h"
#include "Common/ResourceCache.h"
#include "Common/TextureHelper.h"
#include "Core/RenderContext.h"
#include "RenderGraph/RenderGraph.h"

void PostProcess::init() {
    PassBase::init();
    mPipelineLayout = &g_context->getDevice().getResourceCache().requestPipelineLayout(ShaderPipelineKey{
        "full_screen.vert",
        "postprocess/post.frag"});
    mSampler        = &g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);
    tonemapperNames = {"AMD", "DX11", "Reinhard", "Uncharted2", "ACES", "None Tonemapper"};
}
void PostProcess::render(RenderGraph& rg) {
    rg.addGraphicPass(
        "PostProcess",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto input  = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            auto & viewPortImage = rg.getBlackBoard().getHwImage(RENDER_VIEW_PORT_IMAGE_NAME);
            auto outputRT = rg.createTexture("PostProcessInput", RenderGraphTexture::Descriptor{.extent = viewPortImage.getExtent2D(),.useage = TextureUsage::TRANSFER_SRC | TextureUsage::COLOR_ATTACHMENT, .format = viewPortImage.getFormat()});
            builder.readTexture(input, TextureUsage::SAMPLEABLE);
            builder.writeTexture(outputRT, TextureUsage::COLOR_ATTACHMENT);
            RenderGraphPassDescriptor desc{};
            desc.textures = {outputRT};
            desc.addSubpass({{}, {outputRT}});
            builder.declare(desc);
        },
        [&](RenderPassContext& context) {
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthTestEnable = false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
            g_context->bindImageSampler(0, rg.getBlackBoard().getImageView(RENDER_VIEW_PORT_IMAGE_NAME), *mSampler).bindImageSampler(1,TextureHelper::GetBlueNoise256()->getVkImageView(), *mSampler).bindPushConstants(pcPost)
                    .flushAndDraw(context.commandBuffer, 3, 1, 0, 0);
            pcPost.frame_number++;
        });
    rg.addImageCopyPass(rg.getBlackBoard().getHandle("PostProcessInput"), rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
}
void PostProcess::updateGui() {
    PassBase::updateGui();
    ImGui::Begin("PostProcess");
    ImGui::SliderFloat("Bloom Exposure", &pcPost.bloom_exposure, 0.0f, 10.0f);
    ImGui::Combo("Tonemapper", reinterpret_cast<int*>(&pcPost.tone_mapper), tonemapperNames.data(), tonemapperNames.size());
    ImGui::Checkbox("Enable Dither", reinterpret_cast<bool*>(&pcPost.dither));
    ImGui::Checkbox("Enable Gamma Correction", reinterpret_cast<bool*>(&pcPost.gamma_correction));
    ImGui::End();
}
