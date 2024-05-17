#include "SSGIPass.h"

#include "Common/ResourceCache.h"
#include "Core/RenderContext.h"
#include "RenderGraph/RenderGraph.h"
void SSGIPass::render(RenderGraph& rg) {
    rg.addGraphicPass(
        "ssgi", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto& blackBoard = rg.getBlackBoard();
            auto  depth      = blackBoard["depth"];
            auto  normal     = blackBoard["normal"];
            auto  diffuse    = blackBoard["diffuse"];
            auto  emission   = blackBoard["emission"];
            auto  output     = blackBoard.getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            auto  ssgi       = rg.createTexture("ssgi",
                                                {.extent = g_context->getViewPortExtent(),
                                                 .useage = TextureUsage::SAMPLEABLE |
                                                           TextureUsage::COLOR_ATTACHMENT

                                         });
            builder.readTextures({normal, diffuse, emission});
            builder.readTextures({output, depth}, RenderGraphTexture::Usage::SAMPLEABLE);
            builder.writeTexture(ssgi);

            RenderGraphPassDescriptor desc{};
            desc.setTextures({diffuse, normal, emission, ssgi}).addSubpass({.inputAttachments = {diffuse, normal, emission}, .outputAttachments = {ssgi}, .disableDepthTest = true});
            builder.declare(desc);
            // builder.addSubPass();
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer  = context.commandBuffer;
            auto  view           = g_manager->fetchPtr<View>("view");
            auto& blackBoard     = rg.getBlackBoard();
            auto& pipelinelayout = g_context->getDevice().getResourceCache().requestPipelineLayout(std::vector<std::string>{"full_screen.vert", "postprocess/ssgi.frag"});
            g_context->getPipelineState().setPipelineLayout(pipelinelayout).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthTestEnable = false});
            view->bindViewBuffer();
            auto& sampler = g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 1);
            g_context->bindImage(0, blackBoard.getImageView("diffuse"))
                .bindImage(1, blackBoard.getImageView("normal"))
                .bindImage(2, blackBoard.getImageView("emission"))
                .bindImageSampler(3, blackBoard.getImageView("depth"), sampler)
                .bindImageSampler(4, blackBoard.getImageView(RENDER_VIEW_PORT_IMAGE_NAME), sampler)
                .flushAndDraw(commandBuffer, 3, 1, 0, 0);
        });

    rg.addImageCopyPass(rg.getBlackBoard().getHandle("ssgi"), rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
}
void SSGIPass::init() {
    PassBase::init();
}
void SSGIPass::updateGui() {
    PassBase::updateGui();
}
void SSGIPass::update() {
    PassBase::update();
}
