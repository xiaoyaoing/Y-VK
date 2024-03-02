#include "LightInjectionPass.h"

#include "ClipmapRegion.h"
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
    auto& commandBuffer = g_context->getGraphicCommandBuffer();
    rg.addPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {
        auto radiance  = rg.importTexture("radiance", mLightInjectionImage.get());

        builder.writeTexture(radiance);   
        // builder.readTexture(normal);     

        RenderGraphPassDescriptor desc{};
        desc.textures = {radiance};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {}});
            builder.declare(desc); }, [&](RenderPassContext& context) {
                auto clipRegions = g_manager->fetchPtr<std::vector<ClipmapRegion>>("clipmap_regions");
                auto& view       = *g_manager->fetchPtr<View>("view");
                view.bindViewBuffer().bindViewShading();
                    g_context->getPipelineState().setPipelineLayout(*mLightInjectionPipelineLayout);
                                g_context->bindImage(2, rg.getBlackBoard().getHwImage("radiance").getVkImageView(VK_IMAGE_VIEW_TYPE_3D,VK_FORMAT_R32_UINT));
                for(uint32_t i = 0 ;i<CLIP_MAP_LEVEL_COUNT;i++) {
                    // auto buffer = g_manager->fetchPtr<Buffer>("voxel_param_buffer");
                    if(frameIndex % kUpdateRegionLevelOffsets[i] == 0) {
                        auto buffer = g_context->allocateBuffer(sizeof(VoxelizationParamater), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

                        auto & clipRegion =  clipRegions->operator[](i);
                        auto voxelParam =  clipRegion.getVoxelizationParam();
                        buffer.buffer->uploadData(&voxelParam, buffer.size,buffer.offset);
                        g_context->bindBuffer(5, *buffer.buffer,buffer.offset,buffer.size);
                        for(auto& primitive : view.getMVisiblePrimitives()) {
                            if(primitive->getDimensions().overlaps(clipRegion.getBoundingBox()))
                                g_context->bindPrimitiveGeom(commandBuffer,*primitive).flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, 0);
                        }
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
        g_context->getDevice(), std::string("voxelRadianceImage"), imageResolution, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_3D);
    mLightInjectionImage->createImageView(VK_IMAGE_VIEW_TYPE_3D, VK_FORMAT_R32_UINT);
}