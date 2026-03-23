#include "TinyNanite.h"

#include "NaniteBuilder.h"
void TinyNanite::drawFrame(RenderGraph& rg) {
    // rg.addGraphicPass(
    //     "clusterColorPass",
    //     [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
    //         const auto output = rg.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
    //         builder.writeTexture(output, RenderGraphTexture::Usage::COLOR_ATTACHMENT);
    //         auto depth = rg.createTexture(DEPTH_IMAGE_NAME, {.extent = g_context->getViewPortExtent(), .useage = TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE});
    //         builder.writeTexture(depth, RenderGraphTexture::Usage::DEPTH_ATTACHMENT);
    //         builder.declare(RenderGraphPassDescriptor({output, depth}, {.outputAttachments = {output, depth}}));
    //     },
    //     [&](RenderPassContext& context) {
    //         view->bindViewBuffer();
    //         g_context->bindShaders({"tinyNanite/clusterColor.vert", "tinyNanite/clusterColor.frag"});
    //         g_context->bindPrimitiveGeom(context.commandBuffer, *mPrimitive).flushAndDrawIndexed(context.commandBuffer, mPrimitive->indexCount);
    //     });

    // g_context->bindPrimitiveGeom(command*mPrimitive).flushAndDrawIndexed(F
}
void TinyNanite::prepare() {
    Application::prepare();
    auto naniteMeshInput = NaniteBuilder::createNaniteExampleMeshInputData();
    // mPrimitive           = NaniteBuilder::createNaniteExamplePrimitive();
    NaniteBuilder::Build(*naniteMeshInput, nullptr, {});
    scene = std::make_unique<Scene>();
    scene->getLoadCompleteInfo().SetGeometryLoaded();
    scene->getLoadCompleteInfo().SetTextureLoaded();
}
void TinyNanite::onUpdateGUI() {
    Application::onUpdateGUI();
}

MAIN(TinyNanite)
