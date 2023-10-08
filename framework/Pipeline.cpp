#include "Pipeline.h"
#include "CommandBuffer.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include <Device.h>
#include <Subpass.h>
#include <array>

// Pipeline::Pipeline(const PipelineInfo& pipelineInfo, ptr<Device> device,
//                    const std::vector<VkVertexInputBindingDescription>& inputBindings,
//                    const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs,
//                    VkPipelineLayout pipelineLayout, VkRenderPass renderPass)
// {
//     // DVKGfxPipeline* pipeline    = new DVKGfxPipeline();
//     _device = device;
//     _layout = pipelineLayout;
//     VkPipelineVertexInputStateCreateInfo vertexInputState{};
//     vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//     vertexInputState.vertexBindingDescriptionCount = (uint32_t)inputBindings.size();
//     vertexInputState.pVertexBindingDescriptions = inputBindings.data();
//     vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributs.size();
//     vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();
//
//     VkPipelineColorBlendStateCreateInfo colorBlendState{};
//     colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//     colorBlendState.attachmentCount = pipelineInfo.colorAttachmentCount;
//     colorBlendState.pAttachments = pipelineInfo.blendAttachmentStates;
//
//     VkPipelineViewportStateCreateInfo viewportState{};
//     viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//     viewportState.viewportCount = 1;
//     viewportState.scissorCount = 1;
//
//     std::vector<VkDynamicState> dynamicStateEnables;
//     dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
//     dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
//
//     VkPipelineDynamicStateCreateInfo dynamicState{};
//     dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//     dynamicState.dynamicStateCount = 2;
//     dynamicState.pDynamicStates = dynamicStateEnables.data();
//
//     std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
//     pipelineInfo.fillShaderStages(shaderStages);
//
//     VkGraphicsPipelineCreateInfo pipelineCreateInfo;
//     pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//     pipelineCreateInfo.layout = pipelineLayout;
//     pipelineCreateInfo.renderPass = renderPass;
//     pipelineCreateInfo.subpass = pipelineInfo.subpass;
//     pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
//     pipelineCreateInfo.pStages = shaderStages.data();
//     pipelineCreateInfo.pVertexInputState = &vertexInputState;
//     pipelineCreateInfo.pInputAssemblyState = &(pipelineInfo.inputAssemblyState);
//     pipelineCreateInfo.pRasterizationState = &(pipelineInfo.rasterizationState);
//     pipelineCreateInfo.pColorBlendState = &colorBlendState;
//     pipelineCreateInfo.pMultisampleState = &(pipelineInfo.multisampleState);
//     pipelineCreateInfo.pViewportState = &viewportState;
//     pipelineCreateInfo.pDepthStencilState = &(pipelineInfo.depthStencilState);
//     pipelineCreateInfo.pDynamicState = &dynamicState;
//
//     if (pipelineInfo.tessellationState.patchControlPoints != 0)
//     {
//         pipelineCreateInfo.pTessellationState = &(pipelineInfo.tessellationState);
//     }
//
//     // todo add pipeline cache
//     VK_CHECK_RESULT(
//         vkCreateGraphicsPipelines(device->getHandle(), nullptr, 1, &pipelineCreateInfo, nullptr, &_pipeline));
// }

Pipeline::Pipeline(Device& device, const PipelineState& pipelineState): device(device)
{
    VkGraphicsPipelineCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

    auto shaders = pipelineState.getPipelineLayout().getShaders();

    std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;

    std::transform(shaders.begin(), shaders.end(), std::back_inserter(stageCreateInfos), [](const Shader& shader)
    {
        return shader.PipelineShaderStageCreateInfo();
    });

    createInfo.stageCount = toUint32(stageCreateInfos.size());
    createInfo.pStages = stageCreateInfos.data();

    VkPipelineVertexInputStateCreateInfo vertexInputState{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    vertexInputState.pVertexAttributeDescriptions = pipelineState.getVertexInputState().attributes.data();
    vertexInputState.vertexAttributeDescriptionCount = toUint32(pipelineState.getVertexInputState().attributes.size());

    vertexInputState.pVertexBindingDescriptions = pipelineState.getVertexInputState().bindings.data();
    vertexInputState.vertexBindingDescriptionCount = toUint32(pipelineState.getVertexInputState().bindings.size());

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
    };

    inputAssemblyState.topology = pipelineState.getInputAssemblyState().topology;
    inputAssemblyState.primitiveRestartEnable = pipelineState.getInputAssemblyState().primitiveRestartEnable;

    VkPipelineViewportStateCreateInfo viewportState{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

    viewportState.viewportCount = pipelineState.getViewportState().viewportCount;
    viewportState.scissorCount = pipelineState.getViewportState().scissorCount;

    VkPipelineRasterizationStateCreateInfo rasterizationState{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
    };

    rasterizationState.depthClampEnable = pipelineState.getRasterizationState().depthClampEnable;
    rasterizationState.rasterizerDiscardEnable = pipelineState.getRasterizationState().rasterizerDiscardEnable;
    rasterizationState.polygonMode = pipelineState.getRasterizationState().polygonMode;
    rasterizationState.cullMode = pipelineState.getRasterizationState().cullMode;
    rasterizationState.frontFace = pipelineState.getRasterizationState().frontFace;
    rasterizationState.depthBiasEnable = pipelineState.getRasterizationState().depthBiasEnable;
    rasterizationState.depthBiasClamp = 1.0f;
    rasterizationState.depthBiasSlopeFactor = 1.0f;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampleState{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    multisampleState.sampleShadingEnable = pipelineState.getMultisampleState().sampleShadingEnable;
    multisampleState.rasterizationSamples = pipelineState.getMultisampleState().rasterizationSamples;
    multisampleState.minSampleShading = pipelineState.getMultisampleState().minSampleShading;
    multisampleState.alphaToCoverageEnable = pipelineState.getMultisampleState().alphaToCoverageEnable;
    multisampleState.alphaToOneEnable = pipelineState.getMultisampleState().alphaToOneEnable;

    if (pipelineState.getMultisampleState().sampleMask)
    {
        multisampleState.pSampleMask = &pipelineState.getMultisampleState().sampleMask;
    }

    VkPipelineDepthStencilStateCreateInfo depthStencilState{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
    };

    depthStencilState.depthTestEnable = pipelineState.getDepthStencilState().depthTestEnable;
    depthStencilState.depthWriteEnable = pipelineState.getDepthStencilState().depthWriteEnable;
    depthStencilState.depthCompareOp = pipelineState.getDepthStencilState().depthCompareOp;
    depthStencilState.depthBoundsTestEnable = pipelineState.getDepthStencilState().depthBoundsTestEnable;
    depthStencilState.stencilTestEnable = pipelineState.getDepthStencilState().stencilTestEnable;
    depthStencilState.front.failOp = pipelineState.getDepthStencilState().front.failOp;
    depthStencilState.front.passOp = pipelineState.getDepthStencilState().front.passOp;
    depthStencilState.front.depthFailOp = pipelineState.getDepthStencilState().front.depthFailOp;
    depthStencilState.front.compareOp = pipelineState.getDepthStencilState().front.compareOp;
    depthStencilState.front.compareMask = ~0U;
    depthStencilState.front.writeMask = ~0U;
    depthStencilState.front.reference = ~0U;
    depthStencilState.back.failOp = pipelineState.getDepthStencilState().back.failOp;
    depthStencilState.back.passOp = pipelineState.getDepthStencilState().back.passOp;
    depthStencilState.back.depthFailOp = pipelineState.getDepthStencilState().back.depthFailOp;
    depthStencilState.back.compareOp = pipelineState.getDepthStencilState().back.compareOp;
    depthStencilState.back.compareMask = ~0U;
    depthStencilState.back.writeMask = ~0U;
    depthStencilState.back.reference = ~0U;

    VkPipelineColorBlendStateCreateInfo colorBlendState{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

    colorBlendState.logicOpEnable = pipelineState.getColorBlendState().logicOpEnable;
    colorBlendState.logicOp = pipelineState.getColorBlendState().logicOp;
    colorBlendState.attachmentCount = toUint32(pipelineState.getColorBlendState().attachments.size());
    colorBlendState.pAttachments = reinterpret_cast<const VkPipelineColorBlendAttachmentState*>(pipelineState.
        getColorBlendState().attachments.data());
    colorBlendState.blendConstants[0] = 1.0f;
    colorBlendState.blendConstants[1] = 1.0f;
    colorBlendState.blendConstants[2] = 1.0f;
    colorBlendState.blendConstants[3] = 1.0f;

    std::array<VkDynamicState, 9> dynamicStates{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
        VK_DYNAMIC_STATE_DEPTH_BIAS,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS,
        VK_DYNAMIC_STATE_DEPTH_BOUNDS,
        VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
        VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
        VK_DYNAMIC_STATE_STENCIL_REFERENCE,
    };

    VkPipelineDynamicStateCreateInfo dynamicState{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

    dynamicState.pDynamicStates = dynamicStates.data();
    dynamicState.dynamicStateCount = toUint32(dynamicStates.size());

    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;

    createInfo.layout = pipelineState.getPipelineLayout().getHandle();
    createInfo.renderPass = pipelineState.getRenderPass()->getHandle();
    createInfo.subpass = pipelineState.getSubpassIndex();

    //todo add pipeline cache 
    VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.getHandle(), nullptr, 1, &createInfo, nullptr, &_pipeline))
}

void RenderPipeline::draw(CommandBuffer& commandBuffer, VkSubpassContents contents)
{
    std::unique_ptr<Subpass>& pass = subPasses[0];
    // todo handle multpasses
    // pass->updateRenderTargetAttachments(renderFrame.getRenderTarget());
    commandBuffer.beginRenderPass(*renderPass,
                                  Default::clearValues(), contents);
    pass->draw(commandBuffer);
}

RenderPass& RenderPipeline::getRenderPass() const
{
    return *renderPass;
}

void RenderPipeline::addSubPass(std::unique_ptr<Subpass>&& subpass)
{
    subPasses.push_back(std::move(subpass));
}

void RenderPipeline::createRenderPass(RenderTarget& target, std::vector<LoadStoreInfo>& loadStoreOps)
{
    //    assert(subpasses.size() > 0 && "Cannot create a render pass without any subpass");
    assert(!subPasses.empty());
    std::vector<SubpassInfo> subpassInfos(subPasses.size());
    auto subpassInfoIt = subpassInfos.begin();
    for (auto& subpass : subPasses)
    {
        subpassInfoIt->inputAttachments = subpass->getInputAttachments();
        subpassInfoIt->outputAttachments = subpass->getOutputAttachments();
        subpassInfoIt->colorResolveAttachments = subpass->getColorResolveAttachments();
        subpassInfoIt->disableDepthStencilAttachment = subpass->getDisableDepthStencilAttachment();
        subpassInfoIt->depthStencilResolveMode = subpass->getDepthStencilResolveMode();
        subpassInfoIt->depthStencilResolveAttachment = subpass->getDepthStencilResolveAttachment();
        subpassInfoIt->debugName = subpass->getDebugName();

        ++subpassInfoIt;
    }

    // renderPass = std::make_unique<RenderPass>(device, target.getAttachments(), loadStoreOps, subpassInfos);
}

RenderPipeline::RenderPipeline(Device& device) : device(device)
{
}

void RenderPipeline::createRenderPass(VkRenderPass pass)
{
    renderPass = std::make_unique<RenderPass>(device, pass);
}
