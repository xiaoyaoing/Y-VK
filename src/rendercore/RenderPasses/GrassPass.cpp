#include "GrassPass.h"

#include "imgui.h"
#include "Core/RenderContext.h"
#include "Core/View.h"
#include "RenderGraph/RenderGraph.h"

struct GrassPass::PushConstant {
    float grassSpacing = 0.61f;	
    float time = 0;
    int batchCount = 1e5f;
    int  enableLod;
    float windDirection;
    float lodLevel;
    float windScale = 0.15f;

};

void GrassPass::render(RenderGraph& rg) {
    mPushConstant->time += 0.01f;
    rg.addGraphicPass(
        "GrassPass",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            const auto output = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            builder.writeTexture(output, RenderGraphTexture::Usage::COLOR_ATTACHMENT);
            auto  depth = rg.createTexture(DEPTH_IMAGE_NAME, {.extent = g_context->getViewPortExtent(), .useage = TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE});
            builder.writeTexture(depth, RenderGraphTexture::Usage::DEPTH_ATTACHMENT);
            builder.declare(RenderGraphPassDescriptor({output,depth}, {.outputAttachments = {output,depth}}));
        },
        [&](RenderPassContext& context) {
            g_manager->getView()->bindViewBuffer();
            g_context->getPipelineState().setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
            g_context->bindImageSampler(0,perlinNoise->image->getVkImageView(),*perlinNoise->sampler).bindShaders({"meshShader/mgs.mesh", "meshShader/mgs.task", "meshShader/mgs.frag"}).bindPushConstants(*mPushConstant);
            g_context->flushAndDrawMeshTasks(context.commandBuffer, 1, 1, 1);
        });
}
void GrassPass::updateGui() {
    ImGui::SliderFloat("Spacing", &mPushConstant->grassSpacing, 0.1f, 2.0f);
    ImGui::SliderInt("Batch Count", &mPushConstant->batchCount, 1, 1e6f);
    ImGui::Checkbox("Enable LOD", reinterpret_cast<bool*>(&mPushConstant->enableLod));
    ImGui::SliderFloat("Wind Direction", &mPushConstant->windDirection, 0.0f, 1.0f);
    ImGui::SliderFloat("LOD Level", &mPushConstant->lodLevel, 1.f,15.f);
    ImGui::SliderFloat("Wind Scale", &mPushConstant->windScale, 0.1f, 2.0f);
}
GrassPass::GrassPass() {
    mPushConstant = new PushConstant();
    perlinNoise = Texture::loadTextureFromFile(g_context->getDevice(),FileUtils::getResourcePath("perlin_noise.png"));

}