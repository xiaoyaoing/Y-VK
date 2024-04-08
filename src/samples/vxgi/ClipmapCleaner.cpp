#include "ClipmapCleaner.h"

#include "ClipmapRegion.h"
#include "Core/RenderContext.h"
#include "RenderGraph/RenderGraph.h"
struct ClipMapCleaner::ImageCleaningDesc {
    glm::ivec3 regionMinCorner;  // 12
    uint32_t   clipLevel;        // 16
    glm::uvec3 clipMaxExtent;    // 28
    int32_t    clipmapResolution;// 32
    uint32_t   faceCount;        // 36
};

void ClipMapCleaner::clearClipMapRegions(RenderGraph& rg, const ClipmapRegion& clipRegion, RenderGraphHandle imageToClear, uint32_t clipLevel) {
    {
        rg.addComputePass(
            "clear clipmap",
            [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
                builder.writeTexture(imageToClear);
            },
            [clipRegion, imageToClear, clipLevel, &rg](RenderPassContext& context) {
                ImageCleaningDesc desc{.regionMinCorner = clipRegion.minCoord, .clipLevel = clipLevel, .clipMaxExtent = clipRegion.extent, .clipmapResolution = VOXEL_RESOLUTION, .faceCount = 6};
                const glm::uvec3  groupCount = glm::uvec3(glm::ceil(glm::vec3(clipRegion.extent) / 8.0f));
                g_context->getPipelineState().setPipelineLayout(*mPipelineLayout);
                g_context->bindImage(0, rg.getTexture(imageToClear)->getHwTexture()->getVkImageView()).bindPushConstants(desc).dispath(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            });
    }
}
void ClipMapCleaner::init() {
    Device& device  = g_context->getDevice();
    mPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/clearClipmap.comp"});
}
