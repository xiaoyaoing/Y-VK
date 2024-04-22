#include "GBufferPass.h"

#include "Common/ResourceCache.h"
#include "Core/RenderContext.h"
#include "Core/View.h"
void GBufferPass::init() {
    // mNormal = std::make_unique<SgImage>(device,"normal",VKExt, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    Device&             device = g_context->getDevice();
    std::vector<Shader> shaders{
        Shader(device, FileUtils::getShaderPath("defered_one_scene_buffer.vert")),
        Shader(device, FileUtils::getShaderPath("defered_pbr.frag"))};
    mPipelineLayout = std::make_unique<PipelineLayout>(device, shaders);
}
void LightingPass::render(RenderGraph& rg) {
    rg.addGraphicPass(
        "LightingPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto& blackBoard = rg.getBlackBoard();
            auto  depth      = blackBoard["depth"];
            auto  normal     = blackBoard["normal"];
            auto  diffuse    = blackBoard["diffuse"];
            auto  emission   = blackBoard["emission"];
            auto  output     = blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME);

            builder.readTextures({depth, normal, diffuse, emission, output});
            builder.writeTexture(output);

            RenderGraphPassDescriptor desc{};
            desc.setTextures({output, diffuse, depth, normal, emission}).addSubpass({.inputAttachments = {diffuse, depth, normal, emission}, .outputAttachments = {output}, .disableDepthTest = true});
            builder.declare(desc);
            // builder.addSubPass();
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;
            auto  view          = g_manager->fetchPtr<View>("view");
            auto& blackBoard    = rg.getBlackBoard();
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthTestEnable = false});
            view->bindViewBuffer();
            g_context->bindImage(0, blackBoard.getImageView("diffuse"))
                .bindImage(1, blackBoard.getImageView("normal"))
                .bindImage(2, blackBoard.getImageView("emission"))
                .bindImage(3, blackBoard.getImageView("depth"))
                .flushAndDraw(commandBuffer, 3, 1, 0, 0);
        });
}
void LightingPass::init() {
    std::vector<Shader> shaders{
        Shader(g_context->getDevice(), FileUtils::getShaderPath("lighting.vert")),
        Shader(g_context->getDevice(), FileUtils::getShaderPath("lighting_pbr.frag"))};
    mPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), shaders);
}

struct IBLLightingPassPushConstant {
    float exposure        = 4.5f;
    float gamma           = 2.2f;
    float scaleIBLAmbient = 1.0f;
    float prefilteredCubeMipLevels;
    int   debugMode;
    int   padding[3];
};

void IBLLightingPass::render(RenderGraph& rg) {
    rg.addGraphicPass(
        "IBLLightingPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto& blackBoard     = rg.getBlackBoard();
            auto  depth          = blackBoard["depth"];
            auto  normal         = blackBoard["normal"];
            auto  diffuse        = blackBoard["diffuse"];
            auto  emission       = blackBoard["emission"];
            auto  output         = blackBoard.getHandle(SWAPCHAIN_IMAGE_NAME);
            auto  irradianceCube = blackBoard.getHandle("irradianceCube");
            auto  prefilterCube  = blackBoard.getHandle("prefilterCube");
            auto  brdfLUT        = blackBoard.getHandle("brdfLUT");

            builder.readTextures({depth, normal, diffuse, emission, output});
            builder.readTextures({irradianceCube, prefilterCube, brdfLUT}, TextureUsage::SAMPLEABLE);
            builder.writeTexture(output);

            RenderGraphPassDescriptor desc{};
            desc.setTextures({output, diffuse, depth, normal, emission}).addSubpass({.inputAttachments = {diffuse, depth, normal, emission}, .outputAttachments = {output}, .disableDepthTest = true});
            builder.declare(desc);
            // builder.addSubPass();
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;
            auto  view          = g_manager->fetchPtr<View>("view");
            auto& blackBoard    = rg.getBlackBoard();
            g_context->getPipelineState().setPipelineLayout(g_context->getDevice().getResourceCache().requestPipelineLayout(std::vector<std::string>{"lighting.vert", "pbrLab/lighting_ibl.frag"})).setRasterizationState({.cullMode = VK_CULL_MODE_NONE}).setDepthStencilState({.depthTestEnable = false});
            view->bindViewBuffer();

            auto& irradianceCube = blackBoard.getImageView("irradianceCube");
            auto& prefilterCube  = blackBoard.getImageView("prefilterCube");
            auto& brdfLUT        = blackBoard.getImageView("brdfLUT");

            IBLLightingPassPushConstant constant;
            constant.prefilteredCubeMipLevels = prefilterCube.getImage().getMipLevelCount();
            constant.debugMode                = debugMode;
            g_context->bindPushConstants(constant);

            auto& irradianceCubeSampler = g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, irradianceCube.getImage().getMipLevelCount());
            auto& prefilterCubeSampler  = g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, prefilterCube.getImage().getMipLevelCount());
            auto& brdfLUTSampler        = g_context->getDevice().getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);

            g_context->bindImageSampler(0, irradianceCube, irradianceCubeSampler).bindImageSampler(1, prefilterCube, prefilterCubeSampler).bindImageSampler(2, brdfLUT, brdfLUTSampler);

            g_context->bindImage(0, blackBoard.getImageView("diffuse"))
                .bindImage(1, blackBoard.getImageView("normal"))
                .bindImage(2, blackBoard.getImageView("emission"))
                .bindImage(3, blackBoard.getImageView("depth"))
                .flushAndDraw(commandBuffer, 3, 1, 0, 0);
        });
}

enum class DebugMode {
    NONE = 0,
    DIFFUSE,
    SPECULAR,
    NORMAL,
    DEPTH,
    ALBEDO,
    METALLIC,
    ROUGHNESS,
    AMBIENT_OCCLUSION,
    IRRADIANCE,
    PREFILTER,
    BRDF_LUT
};

void IBLLightingPass::updateGui() {
    ImGui::Combo("Debug Mode", &debugMode, "None\0Diffuse\0Specular\0Normal\0Depth\0Albedo\0Metallic\0Roughness\0Ambient Occlusion\0Irradiance\0Prefilter\0BRDF LUT\0");
}

void GBufferPass::render(RenderGraph& rg) {
    auto& blackBoard    = rg.getBlackBoard();
    auto& renderContext = g_context;
    rg.addGraphicPass(
        "GBufferPass", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto diffuse = rg.createTexture("diffuse",
                                            {.extent = renderContext->getSwapChainExtent(),
                                             .useage = TextureUsage::SUBPASS_INPUT |
                                                       TextureUsage::COLOR_ATTACHMENT});
            
            auto normal = rg.createTexture("normal",
                                           {.extent = renderContext->getSwapChainExtent(),
                                            .useage = TextureUsage::SUBPASS_INPUT |
                                                      TextureUsage::COLOR_ATTACHMENT

                                           });

            auto emission = rg.createTexture("emission",
                                             {.extent = renderContext->getSwapChainExtent(),
                                              .useage = TextureUsage::SUBPASS_INPUT |
                                                        TextureUsage::COLOR_ATTACHMENT});

            auto depth = rg.createTexture("depth", {.extent = renderContext->getSwapChainExtent(), .useage = TextureUsage::SUBPASS_INPUT | TextureUsage::DEPTH_ATTACHMENT

                                                   });
            
         RenderGraphPassDescriptor desc({diffuse,  normal, emission, depth}, {.outputAttachments = {diffuse,  normal, emission, depth}});
            builder.declare(desc);

            builder.writeTextures({diffuse,  emission, depth}, TextureUsage::COLOR_ATTACHMENT).writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT); }, [&](RenderPassContext& context) {
            renderContext->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthCompareOp = VK_COMPARE_OP_GREATER});
            g_manager->fetchPtr<View>("view")->bindViewBuffer().bindViewShading().bindViewGeom(context.commandBuffer).drawPrimitives(context.commandBuffer); });
}