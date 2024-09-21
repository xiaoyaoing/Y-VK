#include "PostProcess.h"

#include "Common/ResourceCache.h"
#include "Core/RenderContext.h"
#include "RenderGraph/RenderGraph.h"

void PostProcess::init() {
    PassBase::init();
    mPipelineLayout = &g_context->getDevice().getResourceCache().requestPipelineLayout(std::vector<std::string>{
        "full_screen.vert",
        "postprocess/post.frag"});
    mSampler        = &g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);
    tonemapperNames = {"AMD", "DX11", "Reinhard", "Uncharted2", "ACES", "None Tonemapper"};
}
void PostProcess::render(RenderGraph& rg) {
    rg.addGraphicPass(
        "PostProcess",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto inputRt = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            auto output  = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            builder.readTextures({inputRt}, TextureUsage::SAMPLEABLE);
            builder.writeTexture(output, TextureUsage::COLOR_ATTACHMENT);
            RenderGraphPassDescriptor desc{};
            desc.textures = {inputRt, output};
            desc.addSubpass({{}, {output}});
            builder.declare(desc);
        },
        [&](RenderPassContext& context) {
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthTestEnable = false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
            g_context->bindImageSampler(0, rg.getBlackBoard().getImageView(RENDER_VIEW_PORT_IMAGE_NAME), *mSampler)
                    .bindPushConstants(pcPost)
                    .flushAndDraw(context.commandBuffer, 3, 1, 0, 0);
        });
}
void PostProcess::updateGui() {
    PassBase::updateGui();
    ImGui::Begin("PostProcess");
    ImGui::SliderFloat("Bloom Exposure", &pcPost.bloom_exposure, 0.0f, 10.0f);
    ImGui::Combo("Tonemapper", reinterpret_cast<int*>(&pcPost.tone_mapper), tonemapperNames.data(), tonemapperNames.size());
    ImGui::End();
}
