#include "VoxelizationPass.h"

#include "VxgiCommon.h"
#include "Core/RenderContext.h"
#include "Core/View.h"

void VoxelizationPass::initClipRegions() {
    mBBoxes = g_manager->fetchPtr<std::vector<BBox>>("bboxes");

    mClipRegions.resize(CLIP_MAP_LEVEL_COUNT);
    constexpr float extentWorldLevel0 = 16.f;

    int extent     = VOXEL_RESOLUTION;
    int halfExtent = extent / 2;

    // Define clip regions centered around the origin (0, 0, 0) in voxel coordinates
    for (std::size_t i = 0; i < mClipRegions.size(); ++i) {
        mClipRegions[i].minCoord  = glm::ivec3(-halfExtent);
        mClipRegions[i].extent    = glm::ivec3(extent);
        mClipRegions[i].voxelSize = (extentWorldLevel0 * std::exp2f(static_cast<float>(i))) / extent;
    }

    // Move regions to be "centered" (close to the center in discrete voxel coordinates) around the camera

    for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
        auto& clipRegion = mClipRegions[clipmapLevel];
        // Compute the closest delta in voxel coordinates
        glm::ivec3 delta = computeChangeDeltaV(clipmapLevel);
        clipRegion.minCoord += delta;
    }
}

void VoxelizationPass::init() {
    initClipRegions();

    Device& device = g_context->getDevice();

    mVoxelizationPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/voxelization.vert", "vxgi/voxelization.frag"});

    VkExtent3D imageResolution = {VOXEL_RESOLUTION * 6,
                                  VOXEL_RESOLUTION * CLIP_MAP_LEVEL_COUNT,
                                  VOXEL_RESOLUTION};

    mVoxelizationImage = std::make_unique<SgImage>(
        device, std::string("opacityImage"), imageResolution, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);

    // mVoxelParamBuffer = std::make_unique<Buffer>(device, sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    mVoxelParam = VoxelizationParamater{
        .voxelResolution = VOXEL_RESOLUTION,
    };

    mClipMapCleaner = std::make_unique<ClipMapCleaner>();
    mRevoxelizationRegions.resize(CLIP_MAP_LEVEL_COUNT);

    //  g_manager->putPtr("voxel_param_buffer", mVoxelParamBuffer.get());
    g_manager->putPtr("clipmap_regions", &mClipRegions);

    mFullRevoxelization = true;
}

void VoxelizationPass::render(RenderGraph& rg) {
    updateVoxelization();

    {
        auto opacity = rg.importTexture("opacity", mVoxelizationImage.get());

        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            for (auto& clipRegion : mRevoxelizationRegions[clipmapLevel]) {
                mClipMapCleaner->clearClipMapRegions(rg, clipRegion, opacity, clipmapLevel);
            }
        }
    }

    Blackboard& blackboard    = rg.getBlackBoard();
    auto&       commandBuffer = g_context->getGraphicCommandBuffer();
    rg.addPass(
        "voxelization pass",
        [&](auto& builder, auto& setting) {
            auto opacity = rg.getBlackBoard().getHandle("opacity");
            builder.writeTexture(opacity);
            // builder.writeTexture(radiance);
            RenderGraphPassDescriptor desc({opacity}, {.outputAttachments = {}});
            builder.declare(desc);
        },
        [&](auto& passContext) {
            g_context->bindImage(1, rg.getBlackBoard().getImageView("opacity"));//.bindImage(2, rg.getBlackBoard().getImageView("radiance"));

            for (int i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {

                for (auto& clipRegion : mRevoxelizationRegions[i]) {

                    auto bufferAllocation = g_context->allocateBuffer(sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                    mVoxelParam.clipmapLevel = i;
                    //  mVoxelParam.clipmapMinCoord    = clipRegion.getMaxCoord();
                    //  mVoxelParam.clipmapMaxCoord    = clipRegion.getMinCoord();
                    mVoxelParam.clipmapMinWorldPos = clipRegion.getMinPosWorld();
                    mVoxelParam.clipmapMaxWorldPos = clipRegion.getMaxPosWorld();
                    mVoxelParam.voxelSize          = clipRegion.voxelSize;
                    mVoxelParam.maxExtentWorld     = clipRegion.getExtentWorld().x;
                    mVoxelParam.voxelResolution    = VOXEL_RESOLUTION;
                    bufferAllocation.buffer->uploadData(&mVoxelParam, bufferAllocation.size, bufferAllocation.offset);
                    g_context->bindBuffer(5, *bufferAllocation.buffer, bufferAllocation.offset, bufferAllocation.size);

                    //  mVoxelParamBuffer->uploadData(&mVoxelParam, sizeof(VoxelizationParamater));

                    //  g_context->bindBuffer(5, *mVoxelParamBuffer);

                    g_context->getPipelineState().setPipelineLayout(
                        *mVoxelizationPipelineLayout);
                    auto* view = g_manager->fetchPtr<View>("view");
                    view->bindViewBuffer();
                    for (const auto& primitive : view->getMVisiblePrimitives()) {
                        if (primitive->getDimensions().overlaps(clipRegion.getBoundingBox()))
                            g_context->bindPrimitiveGeom(commandBuffer, *primitive).flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
                    }
                }
            }
        });
}

//Returns the difference between the current BoundingBox and the previous frame clip region in voxel coordinates.
glm::ivec3 VoxelizationPass::computeChangeDeltaV(uint32_t clipmapLevel) {
    const auto& clipRegion = mClipRegions[clipmapLevel];
    const auto& bb         = mBBoxes->at(clipmapLevel);

    glm::ivec3 delta = (bb.min() - clipRegion.getMinPosWorld()) / clipRegion.voxelSize;

    return delta;
}

//see https://zhuanlan.zhihu.com/p/549938187
void VoxelizationPass::fillRevoxelizationRegions(uint32_t clipLevel, const BBox& boundingBox) {
    auto  delta      = computeChangeDeltaV(clipLevel);
    auto& clipRegion = mClipRegions[clipLevel];

    clipRegion.minCoord += delta;

    auto absDelta = glm::abs(delta);
    if (glm::any(glm::greaterThan(absDelta, clipRegion.extent))) {
        mRevoxelizationRegions[clipLevel].emplace_back(clipRegion);
        return;
    }

    // If the change is larger than the minimum change, we need to revoxelize the changed region
    if (absDelta.x > mMinVoxelChange.x) {
        glm::ivec3 newExtent = glm::ivec3(absDelta.x, clipRegion.extent.y, clipRegion.extent.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.x > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
    }

    if (absDelta.y > mMinVoxelChange.y) {
        glm::ivec3 newExtent = glm::ivec3(clipRegion.extent.x, absDelta.y, clipRegion.extent.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.y > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
    }

    if (absDelta.z > mMinVoxelChange.z) {
        glm::ivec3 newExtent = glm::ivec3(clipRegion.extent.x, clipRegion.extent.y, absDelta.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.z > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
    }
}

void VoxelizationPass::updateVoxelization() {
    //update camera bounding box
    // for(uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL; ++clipmapLevel) {
    //     fillRevoxelizationRegions(clipmapLevel, mBBoxes->at(clipmapLevel));
    // }
    mRevoxelizationRegions.clear();
    mRevoxelizationRegions.resize(CLIP_MAP_LEVEL_COUNT, {});

    if (mFullRevoxelization) {
        initClipRegions();
        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            mRevoxelizationRegions[clipmapLevel].emplace_back(mClipRegions[clipmapLevel]);
        }
    } else {
        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            mRevoxelizationRegions[clipmapLevel].clear();
            fillRevoxelizationRegions(clipmapLevel, mBBoxes->at(clipmapLevel));
        }
    }
}

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
            [clipRegion, imageToClear, clipLevel, &rg, this](RenderPassContext& context) {
                ImageCleaningDesc desc{.regionMinCorner = clipRegion.minCoord, .clipLevel = clipLevel, .clipMaxExtent = clipRegion.extent, .clipmapResolution = VOXEL_RESOLUTION, .faceCount = 6};
                const glm::uvec3  groupCount = glm::uvec3(glm::ceil(glm::vec3(clipRegion.extent) / 8.0f));
                g_context->getPipelineState().setPipelineLayout(*mPipelineLayout);
                g_context->bindImage(0, rg.getTexture(imageToClear)->getHwTexture()->getVkImageView()).bindPushConstants(desc).dispath(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            });
    }
}

ClipMapCleaner::ClipMapCleaner() {
    Device& device  = g_context->getDevice();
    mPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/clearClipmap.comp"});
}
