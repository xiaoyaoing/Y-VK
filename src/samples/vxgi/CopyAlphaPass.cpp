#include "CopyAlphaPass.h"

#include "ClipmapCleaner.h"
#include "Core/RenderContext.h"

struct CopyAlphaPassData {
    int uClipLevel;        //  4
    int uClipmapResolution;//  8
    int uFaceCount;        // 12
};

void CopyAlphaPass::render(RenderGraph& rg) {
    rg.addComputePass(
        "Copy Alpha Pass",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            auto opacity  = rg.getBlackBoard().getHandle("opacity");
            auto radiance = rg.getBlackBoard().getHandle("radiance");
            builder.readTextures({opacity, radiance}).writeTexture(radiance);
        },
        [&](RenderPassContext& context) {
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout);
            g_context->bindImageSampler(0, rg.getBlackBoard().getImageView("opacity"), *mSampler).bindImage(0, rg.getBlackBoard().getImageView("radiance"));
            CopyAlphaPassData data{.uClipmapResolution = VxgiConfig::voxelResolution, .uFaceCount = 6};
            for (int i = 0; i < VxgiConfig::clipMapLevelCount; i++) {
                data.uClipLevel     = i;
                uint32_t groupCount = VxgiConfig::voxelResolution / 8;
                g_context->bindPushConstants(data).flushAndDispatch(context.commandBuffer, groupCount, groupCount, groupCount);
            }
        });

    ClipMapCleaner::downSampleRadiace(rg, rg.getBlackBoard().getHandle("radiance"));
}
void CopyAlphaPass::init() {
    mPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), std::vector<std::string>{"vxgi/copyAlpha.comp"});

    mSampler = std::make_unique<Sampler>(g_context->getDevice(), VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 0.0f);
}