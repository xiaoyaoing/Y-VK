#include "ClipmapCleaner.h"

#include "ClipmapRegion.h"
#include "VxgiCommon.h"
#include "Core/RenderContext.h"
#include "RenderGraph/RenderGraph.h"
struct ClipMapCleaner::ImageCleaningDesc {
    glm::ivec3 regionMinCorner;  // 12
    uint32_t   clipLevel;        // 16
    glm::uvec3 clipMaxExtent;    // 28
    int32_t    clipmapResolution;// 32
    uint32_t   faceCount;        // 36
};

struct DownSamplerDesc {
    ivec3 uPrevRegionMinCorner;// 12
    int   uClipLevel;          // 16
    int   uClipmapResolution;  // 20
    int   uDownSampleRegionSize;
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
                g_context->bindImage(0, rg.getTexture(imageToClear)->getHwTexture()->getVkImageView()).bindPushConstants(desc).flushAndDispatch(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            });
    }
}

static  std::string kdownsampleOpacityShader = "vxgi/opacityDownSample.comp";
static  std::string kdownsampleRadianceShader = "vxgi/radianceDownSample.comp";

void ClipMapCleaner::init() {
    Device& device  = g_context->getDevice();
    mPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/clearClipmap.comp"});
}
void ClipMapCleaner::downSampleOpacity(RenderGraph& rg, RenderGraphHandle opacity) {
    if(VxgiContext::getConfig().useDownSample == false) {
        return;
    }
    static DownSamplerDesc desc;
    rg.addComputePass(
        "downsample opacity",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(opacity,RenderGraphTexture::Usage::STORAGE);
        },
        [opacity,&rg](RenderPassContext& context) {
            g_context->bindShaders({kdownsampleOpacityShader}).bindImage(2, rg.getTexture(opacity)->getHwTexture()->getVkImageView());
            const auto& clipmapRegions = VxgiContext::getClipmapRegions();
            for (int i = 1; i < CLIP_MAP_LEVEL_COUNT; ++i) {
                const glm::uvec3 groupCount = glm::uvec3(glm::ceil(glm::vec3(VOXEL_RESOLUTION) / 8.0f));
                desc.uClipLevel             = i;
                desc.uClipmapResolution     = VOXEL_RESOLUTION;
                desc.uPrevRegionMinCorner   = clipmapRegions[i - 1].minCoord;
                desc.uDownSampleRegionSize  = 10;
                g_context->bindPushConstants(desc).flushAndDispatch(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            }
        });
}

void ClipMapCleaner::downSampleRadiace(RenderGraph& rg, RenderGraphHandle radiance) {
    if(VxgiContext::getConfig().useDownSample == false) {
        return;
    }
    static DownSamplerDesc desc;
    rg.addComputePass(
        "downsample radiance",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(radiance,RenderGraphTexture::Usage::STORAGE);
        },
        [radiance,&rg](RenderPassContext& context) {
            g_context->bindShaders({kdownsampleRadianceShader}).bindImage(2, rg.getTexture(radiance)->getHwTexture()->getVkImageView());
            const auto& clipmapRegions = VxgiContext::getClipmapRegions();
            for (int i = 1; i < CLIP_MAP_LEVEL_COUNT; ++i) {
                const glm::uvec3 groupCount = glm::uvec3(glm::ceil(glm::vec3(VOXEL_RESOLUTION) / 8.0f));
                desc.uClipLevel             = i;
                desc.uClipmapResolution     = VOXEL_RESOLUTION;
                desc.uPrevRegionMinCorner   = clipmapRegions[i - 1].minCoord;
                desc.uDownSampleRegionSize  = 10;
                g_context->bindPushConstants(desc).flushAndDispatch(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            }
        });
}
