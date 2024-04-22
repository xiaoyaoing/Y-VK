#include "IBL.h"

#include "Common/ResourceCache.h"
#include "Core/PipelineLayout.h"
#include "Core/RenderContext.h"
#include "Core/math.h"
#include "RenderGraph/RenderGraph.h"
#include <memory>

IBL* g_ibl = nullptr;

static constexpr uint32_t IrradianceCubeDim    = 64;
static constexpr uint32_t PrefilteredEnvMapDim = 512;

struct PrefilteredEnvMapPushConstant {
    float roughness;
    uint  numSamples = 32;
    ivec2 size;
};

struct PrefilteredCubeMapPushConstant {
    float deltaPhi   = (2.0f * float(math::PI)) / 180.0f;
    float deltaTheta = (0.5f * float(math::PI)) / 64.0f;
    uint  mip_level;
    uint  padding;
    ivec2 size;
};

struct BrdfLUTPushConstant {
    float deltaCos;
    float deltaTheta;
    ivec2 size;
};

void IBL::generatePrefilteredCubeMap(RenderGraph& rg) {
    rg.addComputePass(
        "Generate IrradianceCube Map",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(rg.getBlackBoard().getHandle("irradianceCube"), TextureUsage::STORAGE);
            settings.pipelineLayout = cubeMapLayout.get();
        },
        [&](RenderPassContext& context) {
            PrefilteredCubeMapPushConstant constant;
            uint                           mipCount = toUint32(ceil(log2(IrradianceCubeDim)));
            g_context->bindImageSampler(0, environmentCube->getImage().getVkImageView(VK_IMAGE_VIEW_TYPE_CUBE), environmentCube->getSampler());
            for (uint32_t mip = 0; mip < mipCount; ++mip) {
                g_context->bindImage(0, irradianceCube->getVkImageView(VK_IMAGE_VIEW_TYPE_CUBE, irradianceCube->getFormat(), mip, 0, 1, 6));
                ivec2 size         = ivec2(IrradianceCubeDim >> mip, IrradianceCubeDim >> mip);
                constant.size      = size;
                constant.mip_level = mip;
                g_context->bindPushConstants(constant);
                ivec3 groupCount = ivec3((size.x + 8 - 1) / 8, (size.y + 8 - 1) / 8, 6);
                g_context->flushAndDispatch(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            }
        });
}
void IBL::generatePrefilertedEnvMap(RenderGraph& rg) {
    rg.addComputePass(
        "Generate PrefilertedEnvMap",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(rg.getBlackBoard().getHandle("prefilterCube"), TextureUsage::STORAGE);
            settings.pipelineLayout = envMapLayout.get();
        },
        [&](RenderPassContext& context) {
            PrefilteredEnvMapPushConstant constant;
            uint                          mipCount = toUint32(ceil(log2(PrefilteredEnvMapDim)));

            g_context->bindImageSampler(0, environmentCube->getImage().getVkImageView(VK_IMAGE_VIEW_TYPE_CUBE), environmentCube->getSampler());

            for (int mip = 0; mip < mipCount; mip++) {
                constant.roughness = (float)mip / (float)(mipCount - 1);
                constant.size      = ivec2(PrefilteredEnvMapDim >> mip, PrefilteredEnvMapDim >> mip);
                g_context->bindImage(0, prefilterCube->getVkImageView(VK_IMAGE_VIEW_TYPE_CUBE, prefilterCube->getFormat(), mip, 0, 1, 6));
                g_context->bindPushConstants(constant);
                ivec3 groupCount = ivec3((constant.size.x + 8 - 1) / 8, (constant.size.y + 8 - 1) / 8, 6);
                g_context->flushAndDispatch(context.commandBuffer, groupCount.x, groupCount.y, groupCount.z);
            }
        });
}
void IBL::generateBRDFLUT(RenderGraph& rg) {
    rg.addComputePass(
        "Generate BRDFLUT",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(rg.getBlackBoard().getHandle("brdfLUT"), TextureUsage::STORAGE);
            settings.pipelineLayout = brdfLUTLayout.get();
        },
        [&](RenderPassContext& context) {
            VkExtent2D          extent = brdfLUT->getExtent2D();
            BrdfLUTPushConstant constant;
            constant.size       = ivec2(extent.width, extent.height);
            constant.deltaCos   = 1.0f / extent.width;
            constant.deltaTheta = 1.0f / extent.height;
            g_context->bindImage(0, brdfLUT->getVkImageView());
            g_context->bindPushConstants(constant).flushAndDispatch(context.commandBuffer, (extent.width + 8 - 1 / 8), (extent.height + 8 - 1 / 8), 1);
        });
}

void IBL::importTexturesToRenderGraph(RenderGraph& rg) {
    rg.setOutput(rg.importTexture("irradianceCube", irradianceCube));
    rg.setOutput(rg.importTexture("prefilterCube", prefilterCube));
    rg.setOutput(rg.importTexture("brdfLUT", brdfLUT));

    //  rg.importTexture("environmentCube", environmentCube->image.get());
}
IBL::IBL(Device& device, const Texture* texture) : device(g_context->getDevice()), environmentCube(texture) {
    this->brdfLUTLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), std::vector<std::string>{"pbrLab/brdf_lut.comp"});
    this->cubeMapLayout = std::make_unique<PipelineLayout>(g_context->getDevice(), std::vector<std::string>{"pbrLab/irradiance_cube.comp"});
    this->envMapLayout  = std::make_unique<PipelineLayout>(g_context->getDevice(), std::vector<std::string>{"pbrLab/prefiliter_env_map.comp"});

    this->irradianceCube = &device.getResourceCache().requestSgImage("irradianceCube", {IrradianceCubeDim, IrradianceCubeDim, 1}, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_CUBE, VK_SAMPLE_COUNT_1_BIT, toUint32(ceil(log2(IrradianceCubeDim))), 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    this->prefilterCube  = &device.getResourceCache().requestSgImage("prefilterCube", {PrefilteredEnvMapDim, PrefilteredEnvMapDim, 1}, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_CUBE, VK_SAMPLE_COUNT_1_BIT, toUint32(ceil(log2(PrefilteredEnvMapDim))), 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
    this->brdfLUT        = &device.getResourceCache().requestSgImage("brdfLUT", {512, 512, 1}, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D, VK_SAMPLE_COUNT_1_BIT, 1, 1, 0);
}
IBL::~IBL() {
}
void IBL::generate(RenderGraph& rg) {
    // if (generated) return;
    generatePrefilteredCubeMap(rg);
    generatePrefilertedEnvMap(rg);
    generateBRDFLUT(rg);
    generated = true;
}
