#include "VoxelizationPass.h"

#include "ClipmapCleaner.h"
#include "VxgiCommon.h"
#include "imgui.h"
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

    mVoxelParam = VoxelizationParamater{
        .voxelResolution = VOXEL_RESOLUTION,
    };

    g_manager->putPtr("clipmap_regions", &mClipRegions);

    mVoxelParamBuffers.resize(CLIP_MAP_LEVEL_COUNT);
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        mVoxelParamBuffers[i] = std::make_unique<Buffer>(device, sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
}

void VoxelizationPass::render(RenderGraph& rg) {
    updateVoxelization();

    {
        auto opacity = rg.importTexture("opacity", mVoxelizationImage.get());

        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            for (auto& clipRegion : mRevoxelizationRegions[clipmapLevel]) {
                ClipMapCleaner::clearClipMapRegions(rg, clipRegion, opacity, clipmapLevel);
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
        [&](auto& context) {
            g_context->bindImage(1, rg.getBlackBoard().getImageView("opacity")).getPipelineState().setPipelineLayout(*mVoxelizationPipelineLayout);//enableConservativeRasterization(g_context->getDevice().getPhysicalDevice());//.bindImage(2, rg.getBlackBoard().getImageView("radiance"));
            g_manager->fetchPtr<View>("view")->bindViewBuffer().bindViewGeom(context.commandBuffer);
            for (int i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {

                for (auto& clipRegion : mRevoxelizationRegions[i]) {

                    mVoxelParam.clipmapLevel       = i;
                    mVoxelParam.clipmapMinWorldPos = clipRegion.getMinPosWorld();
                    mVoxelParam.clipmapMaxWorldPos = clipRegion.getMaxPosWorld();
                    mVoxelParam.voxelSize          = clipRegion.voxelSize;
                    mVoxelParam.maxExtentWorld     = clipRegion.getExtentWorld().x;
                    mVoxelParam.voxelResolution    = VOXEL_RESOLUTION;

                    mVoxelParamBuffers[i]->uploadData(&mVoxelParam, sizeof(VoxelizationParamater));
                    g_context->bindBuffer(5, *mVoxelParamBuffers[i], 0, sizeof(VoxelizationParamater), 3);
                    g_manager->fetchPtr<View>("view")->drawPrimitives(commandBuffer, [&](const Primitive& primitive) {
                        if(i==0)
                            return clipRegion.getBoundingBox().overlaps(primitive.getDimensions());
                        else {
                            return primitive.getDimensions().overlaps(clipRegion.getBoundingBox(),mClipRegions[i-1].getBoundingBox());
                        } });
                }
            }
        });
}

//Returns the difference between the current BoundingBox and the previous frame clip region in voxel coordinates.
glm::ivec3 VoxelizationPass::computeChangeDeltaV(uint32_t clipmapLevel) {
    const auto& clipRegion = mClipRegions[clipmapLevel];
    const auto& bb         = mBBoxes->at(clipmapLevel);

    //  glm::ivec3 delta = (bb.min() - clipRegion.getMinPosWorld()) / clipRegion.voxelSize;
    glm::vec3 deltaW = bb.min() - clipRegion.getMinPosWorld();

    // The camera needs to move at least the specified minChange amount for the portion to be revoxelized
    float      minChange = clipRegion.voxelSize * 2;
    glm::ivec3 delta     = glm::ivec3(glm::trunc(deltaW / minChange)) * 2;

    // if (clipmapLevel == 0)
    //     LOGI("deltaw delta {} {} {} {} {} {} {} bbmin {} {} {} clipRegionmin {} {} {}", deltaW.x, deltaW.y, deltaW.z, delta.x, delta.y, delta.z, minChange, bb.min().x, bb.min().y, bb.min().z, clipRegion.getMinPosWorld().x, clipRegion.getMinPosWorld().y, clipRegion.getMinPosWorld().z);

    return delta;
}

//see https://zhuanlan.zhihu.com/p/549938187
void VoxelizationPass::fillRevoxelizationRegions(uint32_t clipLevel, const BBox& boundingBox) {
    auto        delta = computeChangeDeltaV(clipLevel);
    const auto& bb    = mBBoxes->at(clipLevel);
    //    LOGI("delta {} {} {} {} {} {}", delta.x, delta.y, delta.z, bb.center().x, bb.center().y, bb.center().z);
    auto& clipRegion = mClipRegions[clipLevel];

    clipRegion.minCoord += delta;
    //  LOGI("delta {} {} {} {} {} {} {}", delta.x, delta.y, delta.z, clipRegion.minCoord.x, clipRegion.minCoord.y, clipRegion.minCoord.z, clipRegion.voxelSize);

    auto absDelta = glm::abs(delta);
    if (glm::any(glm::greaterThan(absDelta, clipRegion.extent))) {
        mRevoxelizationRegions[clipLevel].emplace_back(clipRegion);
        return;
    }

    // If the change is larger than the minimum change, we need to revoxelize the changed region
    if (absDelta.x >= mMinVoxelChange.x) {
        glm::ivec3 newExtent = glm::ivec3(absDelta.x, clipRegion.extent.y, clipRegion.extent.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.x > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
        // clipRegion.minCoord += glm::ivec3(delta.x, 0, 0);
    }

    if (absDelta.y >= mMinVoxelChange.y) {
        glm::ivec3 newExtent = glm::ivec3(clipRegion.extent.x, absDelta.y, clipRegion.extent.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.y > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
        // clipRegion.minCoord += glm::ivec3(0, delta.y, 0);
    }

    if (absDelta.z >= mMinVoxelChange.z) {
        glm::ivec3 newExtent = glm::ivec3(clipRegion.extent.x, clipRegion.extent.y, absDelta.z);
        mRevoxelizationRegions[clipLevel].emplace_back(
            delta.z > 0 ? ((clipRegion.minCoord + glm::ivec3(clipRegion.extent)) - newExtent) : (clipRegion.minCoord),
            newExtent,
            clipRegion.voxelSize);
        //    clipRegion.minCoord += glm::ivec3(0, 0, delta.z);
    }
}

void VoxelizationPass::updateVoxelization() {
    //update camera bounding box
    // for(uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL; ++clipmapLevel) {
    //     fillRevoxelizationRegions(clipmapLevel, mBBoxes->at(clipmapLevel));
    // }
    mRevoxelizationRegions.clear();
    mRevoxelizationRegions.resize(CLIP_MAP_LEVEL_COUNT, {});

    //   mFullRevoxelization = true;

    if (mFullRevoxelization || mFrameIndex++ < 2) {
        initClipRegions();
        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            mRevoxelizationRegions[clipmapLevel].emplace_back(mClipRegions[clipmapLevel]);
        }
        mInitVoxelization = true;
    } else {
        // return;
        for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL_COUNT; ++clipmapLevel) {
            mRevoxelizationRegions[clipmapLevel].clear();
            fillRevoxelizationRegions(clipmapLevel, mBBoxes->at(clipmapLevel));
        }
    }
}
void VoxelizationPass::updateGui() {
    ImGui::Text("clip region 0 min coord: %d %d %d", mClipRegions[0].minCoord.x, mClipRegions[0].minCoord.y, mClipRegions[0].minCoord.z);
    ImGui::Checkbox("Full Revoxelization", &mFullRevoxelization);
}
