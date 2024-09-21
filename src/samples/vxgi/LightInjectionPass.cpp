#include "LightInjectionPass.h"

#include "ClipmapCleaner.h"
#include "ClipmapRegion.h"
#include "imgui.h"
#include "Core/RenderContext.h"
#include "Core/View.h"
#include "Scene/Scene.h"
constexpr uint32_t kUpdateRegionLevelOffsets[CLIP_MAP_LEVEL_COUNT]{
    1,
    1 << 1,
    1 << 2,
    1 << 3,
    1 << 4,
    1 << 5};

void LightInjectionPass::render(RenderGraph& rg) {

    auto radiance = rg.importTexture("radiance", mLightInjectionImage.get());

    auto& commandBuffer = g_context->getGraphicCommandBuffer();
    {
        auto & clipRegions = VxgiContext::getClipmapRegions();
        for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
            // auto buffer = g_manager->fetchPtr<Buffer>("voxel_param_buffer");
            if (frameIndex % kUpdateRegionLevelOffsets[i] == 0 && injectLight[i]) {
                ClipMapCleaner::clearClipMapRegions(rg, clipRegions[i], radiance, i);
            }
        }
    }
    rg.addGraphicPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {

        builder.writeTexture(radiance);   
        // builder.readTexture(normal);     

        RenderGraphPassDescriptor desc{};
        desc.textures = {radiance};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {}});
            builder.declare(desc); }, [&](RenderPassContext& context) {
                // return ;
                auto clipRegions = VxgiContext::getClipmapRegions();
                auto& view       = *g_manager->fetchPtr<View>("view");
                g_context->getPipelineState().setPipelineLayout(*mLightInjectionPipelineLayout);//.enableConservativeRasterization(g_context->getDevice().getPhysicalDevice());
                view.bindViewBuffer().bindViewShading().bindViewGeom(context.commandBuffer);
                g_context->bindImage(2, rg.getBlackBoard().getHwImage("radiance").getVkImageView(VK_IMAGE_VIEW_TYPE_3D,VK_FORMAT_R32_UINT)).bindImage(0,rg.getBlackBoard().getHwImage("diffuse").getVkImageView());
                for(uint32_t i = 0 ;i<CLIP_MAP_LEVEL_COUNT;i++) {
                    // auto buffer = g_manager->fetchPtr<Buffer>("voxel_param_buffer");
                    if(frameIndex % kUpdateRegionLevelOffsets[i] == 0 &&  injectLight[i]) {
                        
                        auto & clipRegion =  clipRegions[i];
                        auto voxelParam =  clipRegion.getVoxelizationParam();
                        voxelParam.clipmapLevel = i;
                        
                        mVoxelParamBuffers[i]->uploadData(&voxelParam, sizeof(VoxelizationParamater));
                        g_context->bindBuffer(5, *mVoxelParamBuffers[i], 0, sizeof(VoxelizationParamater),3).bindPushConstants(frameIndex);
                        g_manager->fetchPtr<View>("view")->drawPrimitives(commandBuffer, [&](const Primitive& primitive) {
                            if(i==0)
                                return clipRegion.getBoundingBox().overlaps(primitive.getDimensions());
                            else {
                                return primitive.getDimensions().overlaps(clipRegion.getBoundingBox(),clipRegions[i-1].getBoundingBox());
                            }
                        });

                    }
                }
     frameIndex++; });
}
void LightInjectionPass::init() {
    VkExtent3D imageResolution    = {VOXEL_RESOLUTION * 6,
                                     VOXEL_RESOLUTION * CLIP_MAP_LEVEL_COUNT,
                                     VOXEL_RESOLUTION};
    mLightInjectionPipelineLayout = std::make_unique<PipelineLayout>(
        g_context->getDevice(), std::vector<std::string>{"vxgi/lightInjection.vert", "vxgi/lightInjection.frag"});
    mLightInjectionImage = std::make_unique<SgImage>(
        g_context->getDevice(), std::string("voxelRadianceImage"), imageResolution, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D, VK_SAMPLE_COUNT_1_BIT, 1, 1, VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT);
    mLightInjectionImage->createImageView(VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R32_UINT);
    
    mVoxelParamBuffers.resize(CLIP_MAP_LEVEL_COUNT);
    for (uint32_t i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
        mVoxelParamBuffers[i] = std::make_unique<Buffer>(g_context->getDevice(), sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
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
    // ImGui::SameLine();
}