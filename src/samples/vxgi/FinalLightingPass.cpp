#include "FinalLightingPass.h"

#include "ClipmapRegion.h"
#include "imgui.h"
#include "Core/RenderContext.h"
#include "Core/View.h"

// struct FinalLightingParam {
//     vec3  volume_center;			 // 12
//     uint32_t  uRenderingMode;			 // 16
//     float voxel_size;				 // 20
//     float clip_map_resoultion;			 // 24
//     float uTraceStartOffset; 		 // 28
//     float uIndirectDiffuseIntensity; // 32
//     float uAmbientOcclusionFactor; 	 // 36
//     float uMinTraceStepFactor; 		 // 40
//     float uIndirectSpecularIntensity;// 44
//     float uOcclusionDecay; 			 // 48
//     int   uEnable32Cones;
// };

void FinalLightingPass::init() {
    std::vector<Shader> shaders = {
        Shader(g_context->getDevice(), FileUtils::getShaderPath("vxgi/voxelConeTracing.vert")),
        Shader(g_context->getDevice(), FileUtils::getShaderPath("vxgi/voxelConeTracing.frag")),
    };
    mFinalLightingPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), shaders);
    mRadianceMapSampler          = std::make_unique<Sampler>(g_context->getDevice(), VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 0.0f);
}
void FinalLightingPass::render(RenderGraph& rg) {
    rg.addPass(
        "Final Lighting Pass",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto& blackBoard = rg.getBlackBoard();
            auto  radiance   = blackBoard.getHandle("radiance");
            auto  diffuse    = blackBoard.getHandle("diffuse");
            //    auto  specular   = blackBoard.getHandle("specular");
            auto normal   = blackBoard.getHandle("normal");
            auto depth    = blackBoard.getHandle("depth");
            auto emission = blackBoard.getHandle("emission");

            auto output = blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME);

            builder.readTextures({radiance, diffuse, normal, depth, emission}).writeTexture(output);

            RenderGraphPassDescriptor desc({radiance, diffuse, normal, depth, emission, output}, {.inputAttachments = {diffuse, normal, depth, emission}, .outputAttachments = {output}});
            builder.declare(desc);
        },
        [&](RenderPassContext& context) {
            auto& view = *g_manager->fetchPtr<View>("view");
            view.bindViewBuffer().bindViewShading();

            auto& blackBoard  = rg.getBlackBoard();
            auto& radianceMap = blackBoard.getHwImage("radiance");

            g_context->getPipelineState().setPipelineLayout(*mFinalLightingPipelineLayout).setDepthStencilState({.depthTestEnable = false}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
            g_context->bindImage(0, blackBoard.getImageView("diffuse"))
                //  .bindImage(1, blackBoard.getImageView("specular"))
                .bindImage(1, blackBoard.getImageView("normal"))
                .bindImage(2, blackBoard.getImageView("emission"))
                .bindImage(3, blackBoard.getImageView("depth"))
                .bindImageSampler(0, radianceMap.getVkImageView(), *mRadianceMapSampler);
            pushFinalLightingParam();
            g_context->flushAndDraw(context.commandBuffer, 3, 1, 0, 0);
            // g_context->bindImage()
            //todo
        });
}

void FinalLightingPass::pushFinalLightingParam() {
    auto& region = g_manager->fetchPtr<std::vector<ClipmapRegion>>("clipmap_regions")->operator[](0);

    mVoxelParam.volume_center       = region.getBoundingBox().center();
    mVoxelParam.voxel_size          = region.voxelSize;
    mVoxelParam.clip_map_resoultion = VOXEL_RESOLUTION;

    g_context->bindPushConstants(mVoxelParam);
}

void FinalLightingPass::updateGui() {
    ImGui::InputInt("Direct intensity", &mVoxelParam.uDirectLighting);
    ImGui::InputInt("Indirect intensity", &mVoxelParam.uIndirectLighting);
}
