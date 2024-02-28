#include "FinalLightingPass.h"

#include "Core/RenderContext.h"
void FinalLightingPass::init() {
    std::vector<Shader> shaders = {
        Shader(g_context->getDevice(), FileUtils::getShaderPath("vxgi/voxelConeTracing.vert")),
        Shader(g_context->getDevice(), FileUtils::getShaderPath("vxgi/voxelConeTracing.frag")),
    };
    mFinalLightingPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), shaders);
}
void FinalLightingPass::render(RenderGraph& rg) {
    auto& blackBoard = rg.getBlackBoard();
    rg.addPass(
        "Final Lighting Pass",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto radiance = blackBoard.getHandle("radiance");
            auto diffuse  = blackBoard.getHandle("diffuse");
            auto specular = blackBoard.getHandle("specular");
            auto normal   = blackBoard.getHandle("normal");
            auto depth    = blackBoard.getHandle("depth");
            auto emission = blackBoard.getHandle("emission");

            auto output = blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME);

            builder.readTextures({ radiance, diffuse, specular, normal, depth, emission}).writeTexture(output);

            RenderGraphPassDescriptor desc({radiance, diffuse, specular, normal, depth, emission,output}, {.inputAttachments = {diffuse, specular, normal, depth, emission}, .outputAttachments = {output}});
            builder.declare(desc);
        },
        [&](RenderPassContext& context) {
            // g_context->bindImage()
            //todo
        });
}