#include "LightInjectionPass.h"

#include "ClipmapCleaner.h"
#include "ClipmapRegion.h"
#include "imgui.h"
#include "Common/VkCommon.h"
#include "Core/RenderContext.h"
#include "Core/View.h"
#include "Core/math.h"
#include "Scene/Scene.h"
// constexpr uint32_t kUpdateRegionLevelOffsets[CLIP_MAP_LEVEL_COUNT]{
//     1,
//     1 << 1,
//     1 << 2,
//     1 << 3,
//     1 << 4,
//     1 << 5
// };

constexpr uint32_t kUpdateRegionLevelOffsets[CLIP_MAP_LEVEL_COUNT]{
    1,
    1 << 0,
    1 << 0,
    1 << 0,
    1 << 0,
    1 << 0
};

struct ViewPortMatrix {
    mat4 uViewProj[3];
    mat4 uViewProjIt[3];
};

static std::pair<int, int> getFirstAndLastLevelTOUpdate(int frameIndex) {
    int firstLevel = -1;
    int lastLevel  = -1;
    for (int i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        if (frameIndex % kUpdateRegionLevelOffsets[i] == 0) {
            if (firstLevel == -1) {
                firstLevel = i;
            }
            lastLevel = i;
        }
    }
    return {firstLevel, lastLevel};
}

void LightInjectionPass::render(RenderGraph& rg) {

    auto radiance = rg.importTexture("radiance", mLightInjectionImage.get());

    {
        auto& clipRegions = VxgiContext::getClipmapRegions();
        for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
            if (frameIndex % kUpdateRegionLevelOffsets[i] == 0 && injectLight[i]) {
                ClipMapCleaner::clearClipMapRegions(rg, clipRegions[i], radiance, i);
            }
        }
    }

    auto [firstLevel, lastLevel] = getFirstAndLastLevelTOUpdate(frameIndex);

    rg.addGraphicPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {

        builder.writeTexture(radiance);   

        RenderGraphPassDescriptor desc{};
        desc.textures = {radiance};
        desc.extent2D = {VOXEL_RESOLUTION   * 1, VOXEL_RESOLUTION   * 1};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {}});
            builder.declare(desc); }, [this, firstLevel, lastLevel, &rg](RenderPassContext& context) {
                auto & commandBuffer = context.commandBuffer;

                auto & clipRegions = VxgiContext::getClipmapRegions();
                auto& view       = *g_manager->fetchPtr<View>("view");
                g_context->getPipelineState().setPipelineLayout(*mLightInjectionPipelineLayout);
                view.bindViewBuffer().bindViewShading().bindViewGeom(context.commandBuffer);
                g_context->bindImage(2, rg.getBlackBoard().getHwImage("radiance").getVkImageView(VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R32_UINT));
                for(int i = 0 ;i<CLIP_MAP_LEVEL_COUNT;i++) {
                    if(frameIndex % kUpdateRegionLevelOffsets[i] == 0 &&  injectLight[i]) {
                        auto & clipRegion =  clipRegions[i];
                        auto voxelParam =  clipRegion.getVoxelizationParam(i == 0 ? nullptr : &clipRegions[i - 1]);
                        voxelParam.clipmapLevel = i;
                        mVoxelParamBuffers[i]->uploadData(&voxelParam, sizeof(VoxelizationParamater));
                        g_context->bindBuffer(5, *mVoxelParamBuffers[i], 0, sizeof(VoxelizationParamater),3).bindPushConstants(glm::ivec2(frameIndex,maxIteration));
                        if(i == firstLevel)
                            VxgiContext::SetVoxelzationViewPortPipelineState(commandBuffer, clipRegion.extent * 1);
                        g_context->bindBuffer(1, * VxgiContext::GetVoxelProjectionBuffer(i), 0, sizeof(ViewPortMatrix), 3);
                        g_manager->fetchPtr<View>("view")->drawPrimitives(commandBuffer, [&](const Primitive& primitive) {
                               return clipRegion.getBoundingBox().overlaps(primitive.getDimensions());
                       });
                        if(i == lastLevel)
                            g_context->resetViewport(commandBuffer);


                    }
                }
     frameIndex++; });
}
void LightInjectionPass::init() {
    VkExtent3D imageResolution    = {VOXEL_RESOLUTION * 6,
                                     VOXEL_RESOLUTION * CLIP_MAP_LEVEL_COUNT,
                                     VOXEL_RESOLUTION};
    mLightInjectionPipelineLayout = std::make_unique<PipelineLayout>(
        g_context->getDevice(), ShaderPipelineKey{"vxgi/lightInjection.vert", "vxgi/lightInjection.geom", "vxgi/lightInjection.frag"});
    mLightInjectionImage = std::make_unique<SgImage>(
        g_context->getDevice(), std::string("voxelRadianceImage"), imageResolution, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D, VK_SAMPLE_COUNT_1_BIT, 1, 1, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT);
    mLightInjectionImage->createImageView(VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R32_UINT);

    mVoxelParamBuffers.resize(CLIP_MAP_LEVEL_COUNT);
    mVoxelViewProjBuffer.resize(CLIP_MAP_LEVEL_COUNT);
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        mVoxelParamBuffers[i]   = std::make_unique<Buffer>(g_context->getDevice(), sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        mVoxelViewProjBuffer[i] = std::make_unique<Buffer>(g_context->getDevice(), sizeof(ViewPortMatrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
}
void LightInjectionPass::updateGui() {
    ImGui::Checkbox("L1", &injectLight[0]);
    ImGui::SameLine();
    ImGui::Checkbox("L2", &injectLight[1]);
    ImGui::SameLine();
    ImGui::Checkbox("L3", &injectLight[2]);
    ImGui::SameLine();
    ImGui::Checkbox("L4", &injectLight[3]);
    ImGui::SameLine();
    ImGui::Checkbox("L5", &injectLight[4]);
    ImGui::SameLine();
    ImGui::Checkbox("L6", &injectLight[5]);

    ImGui::SliderInt("Max Iteration", (int*)&maxIteration, 1, 1000);
    // ImGui::SameLine();
}
