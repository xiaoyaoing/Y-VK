#include "GBufferPass.h"

#include "Core/RenderContext.h"
#include "Core/View.h"
void GBufferPass::init() {
    // mNormal = std::make_unique<SgImage>(device,"normal",VKExt, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    Device&             device = g_context->getDevice();
    std::vector<Shader> shaders{
        Shader(device, FileUtils::getShaderPath("defered_one_scene_buffer.vert")),
        Shader(device, FileUtils::getShaderPath("defered_pbr.frag"))};
    mPipelineLayout = std::make_unique<PipelineLayout>(device, shaders);
}
void LightingPass::render(RenderGraph& rg) {
    rg.addPass(
        "LightingPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto& blackBoard = rg.getBlackBoard();
            auto  depth      = blackBoard["depth"];
            auto  normal     = blackBoard["normal"];
            auto  diffuse    = blackBoard["diffuse"];
            auto  output     = blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME);

            builder.readTextures({depth, normal, diffuse});
            builder.writeTexture(output);

            RenderGraphPassDescriptor desc{};
            desc.setTextures({output, diffuse, depth, normal}).addSubpass({.inputAttachments = {diffuse, depth, normal}, .outputAttachments = {output}, .disableDepthTest = true});
            builder.declare(desc);
            // builder.addSubPass();
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;
            auto  view          = g_manager->fetchPtr<View>("view");
            auto& blackBoard    = rg.getBlackBoard();
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthTestEnable = false});
            view->bindViewBuffer();
            g_context->bindImage(0, blackBoard.getImageView("diffuse"))
                .bindImage(1, blackBoard.getImageView("depth"))
                .bindImage(2, blackBoard.getImageView("normal"))
                .flushAndDraw(commandBuffer, 3, 1, 0, 0);
        });
}
void LightingPass::init() {
    std::vector<Shader> shaders{
        Shader(g_context->getDevice(), FileUtils::getShaderPath("lighting.vert")),
        Shader(g_context->getDevice(), FileUtils::getShaderPath("lighting_pbr.frag"))};
    mPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), shaders);
}

void GBufferPass::render(RenderGraph& rg) {
    auto& blackBoard    = rg.getBlackBoard();
    auto& renderContext = g_context;
    rg.addPass(
        "GBufferPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto diffuse = rg.createTexture("diffuse",
                                            {.extent = renderContext->getSwapChainExtent(),
                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                       TextureUsage::COLOR_ATTACHMENT});
            
            auto normal = rg.createTexture("normal",
                                           {.extent = renderContext->getSwapChainExtent(),
                                            .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT

                                           });

            auto emission = rg.createTexture("emission",
                                             {.extent = renderContext->getSwapChainExtent(),
                                              .useage = TextureUsage::SUBPASS_INPUT |
                                                        TextureUsage::COLOR_ATTACHMENT});

            auto depth = rg.createTexture("depth", {.extent = renderContext->getSwapChainExtent(), .useage = TextureUsage::SUBPASS_INPUT | TextureUsage::DEPTH_ATTACHMENT

                                                   });
            
         RenderGraphPassDescriptor desc({diffuse,  normal, emission, depth}, {.outputAttachments = {diffuse,  normal, emission, depth}});
            builder.declare(desc);

            builder.writeTextures({diffuse,  emission, depth}, TextureUsage::COLOR_ATTACHMENT).writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT); }, [&](RenderPassContext& context) {
            renderContext->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthCompareOp = VK_COMPARE_OP_GREATER});
            g_manager->fetchPtr<View>("view")->bindViewBuffer().bindViewShading().bindViewGeom(context.commandBuffer).drawPrimitives(context.commandBuffer); });
}