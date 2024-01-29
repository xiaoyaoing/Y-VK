#include "VoxelizationPass.h"

#include "VxgiCommon.h"
#include "Core/RenderContext.h"
#include "Core/View.h"

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

    Device& device = g_context->getDevice();

    mVoxelizationPipelineLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"vxgi/voxelization.vert", "vxgi/voxelization.frag"});

    VkExtent3D imageResolution = {VOXEL_RESOLUTION * 6,
                                  VOXEL_RESOLUTION * CLIP_MAP_LEVEL,
                                  VOXEL_RESOLUTION};

    mVoxelizationImage = std::make_unique<SgImage>(
        device, std::string("opacityImage"), imageResolution, VK_FORMAT_R32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);
    mVoxelRadianceImage = std::make_unique<SgImage>(
        device, std::string("voxelRadianceImage"), imageResolution, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);
    mVoxelParamBuffer = std::make_unique<Buffer>(device, sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    mVoxelParam       = VoxelizationParamater{
        VOXEL_RESOLUTION,
        0,
        0.f,
    };
}

void VoxelizationPass::render(RenderGraph& rg) {
    RenderContext* renderContext = g_context;
    Blackboard&    blackboard    = rg.getBlackBoard();
    const auto*    view          = g_manager->fetchPtr<View>("view");
    auto&          commandBuffer = renderContext->getGraphicCommandBuffer();
    rg.addPass(
        "voxelization pass",
        [&](auto& builder, auto& setting) {
            auto opacity = rg.importTexture("opacity", mVoxelizationImage.get());
            builder.writeTexture(opacity);
            RenderGraphPassDescriptor desc{};
            desc.addSubpass({});
            builder.declare("voxelization", desc);
        },
        [&](auto& passContext) {
            renderContext->bindImage(1, mVoxelizationImage->getVkImageView()).bindImage(2, mVoxelRadianceImage->getVkImageView());

            for (int i = 0; i < CLIP_MAP_LEVEL; i++) {
                mVoxelParam.clipmapLevel   = i;
                mVoxelParam.clipmapMaxPos  = mClipRegions[i].getMaxPos();
                mVoxelParam.clipmapMinPos  = mClipRegions[i].minPos;
                mVoxelParam.voxelSize      = mClipRegions[i].voxelSize;
                mVoxelParam.maxExtentWorld = mClipRegions[i].getExtentWorld().x;
                mVoxelParamBuffer->uploadData(&mVoxelParam, sizeof(VoxelizationParamater));

                renderContext->getPipelineState().setPipelineLayout(
                    *mVoxelizationPipelineLayout);
                for (const auto& primitive : view->getMVisiblePrimitives()) {
                    g_context->bindPrimitive(commandBuffer, *primitive).bindMaterial(primitive->material);
                    renderContext->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
                }
            }
        });
}

glm::ivec3 VoxelizationPass::computeChangeDeltaV(uint32_t clipmapLevel) {
    const auto& clipRegion = mClipRegions[clipmapLevel];
    const auto& bb         = mBBoxes->at(clipmapLevel);

    glm::ivec3 delta = (bb.min() - clipRegion.getMinPosWorld()) / clipRegion.voxelSize;

    return delta;
}