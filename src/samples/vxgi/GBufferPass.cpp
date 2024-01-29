#include "GBufferPass.h"

#include "Core/RenderContext.h"
#include "Core/View.h"
void GBufferPass::init() {
    // mNormal = std::make_unique<SgImage>(device,"normal",VKExt, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    std::vector<Shader> shaders{
        Shader(device, FileUtils::getShaderPath("defered.vert")),
        Shader(device, FileUtils::getShaderPath("defered.frag"))};
    mPipelineLayout = std::make_unique<PipelineLayout>(device, shaders);
}

void GBufferPass::render(RenderGraph& rg) {
    auto& blackBoard    = rg.getBlackBoard();
    auto& renderContext = g_context;
    View* view;
    rg.addPass(
        "GBufferPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                    auto albedo = rg.createTexture("albedo",
                                                      {
                                                              .extent = renderContext->getSwapChainExtent(),
                                                              .useage = TextureUsage::SUBPASS_INPUT |
                                                                        TextureUsage::COLOR_ATTACHMENT
                                                      });

                    auto normal = rg.createTexture("normal",
                                                      {
                                                              .extent = renderContext->getSwapChainExtent(),
                                                              .useage = TextureUsage::SUBPASS_INPUT |
                                                                        TextureUsage::COLOR_ATTACHMENT

                                                      });
                    
                    auto depth = rg.createTexture("depth", {
                            .extent = renderContext->getSwapChainExtent(),
                            .useage = TextureUsage::SUBPASS_INPUT |
                                      TextureUsage::DEPTH_ATTACHMENT

                    });

                    RenderGraphPassDescriptor desc{.textures = {depth, albedo, depth, normal}};
                    builder.declare("GBuffer", desc);


                    normal = builder.writeTexture(normal, TextureUsage::COLOR_ATTACHMENT);
                    albedo = builder.writeTexture(albedo, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::COLOR_ATTACHMENT);
                    depth = builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);
                    depth = builder.readTexture(depth, TextureUsage::DEPTH_ATTACHMENT);


                    blackBoard.put("albedo", albedo);
                    blackBoard.put("normal", normal);
                    blackBoard.put("depth", depth); }, [&](RenderPassContext& context) {
                    //   renderContext->beginRenderPass(commandBuffer, context.renderTarget, {});
                    renderContext->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthCompareOp =  VK_COMPARE_OP_GREATER});

                    for(const auto & primitive : view->getMVisiblePrimitives()) {
                        const auto allocation = renderContext->allocateBuffer(
                                sizeof(PerPrimitiveUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                        PerPrimitiveUniform perPrimitiveUniform{.model =  primitive->matrix};
                        allocation.buffer->uploadData(&perPrimitiveUniform, allocation.size, allocation.offset);

                        renderContext->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::PER_RENDERABLE), *allocation.buffer, allocation.offset, allocation.size, 0,0)
                                .bindPrimitive(context.commandBuffer, *primitive)
                                .flushAndDrawIndexed(context.commandBuffer, primitive->indexCount, 1, 0, 0, 0);
                    } });
}