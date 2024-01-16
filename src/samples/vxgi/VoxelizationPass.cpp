#include "VoxelizationPass.h"

#include "VxgiCommon.h"
#include "Core/RenderContext.h"

void VoxelizationPass::init() {

    mClipRegions.resize(CLIP_MAP_LEVEL);
    constexpr float extentWorldLevel0 = 16.f;

    int extent     = VOXEL_RESOLUTION;
    int halfExtent = extent / 2;

    // Define clip regions centered around the origin (0, 0, 0) in voxel coordinates
    for (std::size_t i = 0; i < mClipRegions.size(); ++i) {
        mClipRegions[i].minPos    = glm::ivec3(-halfExtent);
        mClipRegions[i].extent    = glm::ivec3(extent);
        mClipRegions[i].voxelSize = (extentWorldLevel0 * std::exp2f(static_cast<float>(i))) / extent;
    }

    // Move regions to be "centered" (close to the center in discrete voxel coordinates) around the camera

    for (uint32_t clipmapLevel = 0; clipmapLevel < CLIP_MAP_LEVEL; ++clipmapLevel) {
        auto& clipRegion = mClipRegions[clipmapLevel];
        // Compute the closest delta in voxel coordinates
        glm::ivec3 delta = computeChangeDeltaV(clipmapLevel);
        clipRegion.minPos += delta;
    }

    Device& device = VxgiContext::getInstance().device;

    mVoxelizationPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/voxelization.vert", "vxgi/voxelization.frag"});

    VkExtent3D imageResolution = {VOXEL_RESOLUTION * 6,
                                  VOXEL_RESOLUTION * CLIP_MAP_LEVEL,
                                  VOXEL_RESOLUTION};

    mVoxelizationImage = std::make_unique<SgImage>(
        device, std::string("opacityImage"), imageResolution, VK_FORMAT_R32_UINT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);
}

void VoxelizationPass::render(RenderGraph& rg) {
    RenderContext* renderContext = RenderContext::g_context;
    Blackboard&    blackboard    = rg.getBlackBoard();
    const auto*    scene         = gManager->fetchPtr<Scene>("scene");
    auto&          commandBuffer = renderContext->getGraphicCommandBuffer();

    rg.addPass(
        "voxelization pass",
        [&](auto& builder, auto& setting) {
            auto opacity = rg.importTexture("opacity", mVoxelizationImage.get());
            blackboard.put("opacity", opacity);
            builder.writeTexture(opacity);
            RenderGraphPassDescriptor desc{.textures = {opacity}};
            desc.addSubpass({.outputAttachments = {opacity}});
            builder.declare(desc);
        },
        [&](auto& passContext) {
            renderContext->getPipelineState().setPipelineLayout(
                *mVoxelizationPipelineLayout);
            renderContext->bindImage(0, blackboard.getImageView("opacity"));
            for (const auto& primitive : scene->getPrimitives()) {
                renderContext->bindPrimitive(commandBuffer, *primitive)
                    .flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
            }
        });
}

glm::ivec3 VoxelizationPass::computeChangeDeltaV(uint32_t clipmapLevel) {
    const auto& clipRegion = mClipRegions[clipmapLevel];
    const auto& bb         = mBBoxes->at(clipmapLevel);

    glm::ivec3 delta = (bb.min() - clipRegion.getMinPosWorld()) / clipRegion.voxelSize;

    return delta;
}