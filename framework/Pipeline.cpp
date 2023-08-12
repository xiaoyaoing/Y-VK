#include "Pipeline.h"
#include "CommandBuffer.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include <Device.h>
#include <Subpass.h>

Pipeline::Pipeline(const PipelineInfo &pipelineInfo, ptr<Device> device,
                   const std::vector<VkVertexInputBindingDescription> &inputBindings,
                   const std::vector<VkVertexInputAttributeDescription> &vertexInputAttributs,
                   VkPipelineLayout pipelineLayout, VkRenderPass renderPass)
{

    // DVKGfxPipeline* pipeline    = new DVKGfxPipeline();
    _device = device;
    _layout = pipelineLayout;
    VkPipelineVertexInputStateCreateInfo vertexInputState{};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = (uint32_t)inputBindings.size();
    vertexInputState.pVertexBindingDescriptions = inputBindings.data();
    vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributs.size();
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

    VkPipelineColorBlendStateCreateInfo colorBlendState{};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = pipelineInfo.colorAttachmentCount;
    colorBlendState.pAttachments = pipelineInfo.blendAttachmentStates;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    pipelineInfo.fillShaderStages(shaderStages);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = pipelineInfo.subpass;
    pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
    pipelineCreateInfo.pStages = shaderStages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &(pipelineInfo.inputAssemblyState);
    pipelineCreateInfo.pRasterizationState = &(pipelineInfo.rasterizationState);
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &(pipelineInfo.multisampleState);
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &(pipelineInfo.depthStencilState);
    pipelineCreateInfo.pDynamicState = &dynamicState;

    if (pipelineInfo.tessellationState.patchControlPoints != 0)
    {
        pipelineCreateInfo.pTessellationState = &(pipelineInfo.tessellationState);
    }

    // todo add pipeline cache
    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(device->getHandle(), nullptr, 1, &pipelineCreateInfo, nullptr, &_pipeline));
}

void RenderPipeline::draw(CommandBuffer &commandBuffer, RenderTarget &renderTarget, VkSubpassContents contents)
{

    std::unique_ptr<Subpass> &pass = subPasses[0];
    // todo handle multpasses
    pass->updateRenderTargetAttachments(renderTarget);
    commandBuffer.beginRenderPass(renderTarget, *renderPass, RenderContext::g_context->getFrameBuffer(),
                                  Default::clearValues(), contents);
    pass->draw(commandBuffer);
}

RenderPass &RenderPipeline::getRenderPass() const
{
    return *renderPass;
}

void RenderPipeline::addSubPass(std::unique_ptr<Subpass> &&subpass)
{
    subPasses.push_back(std::move(subpass));
}

void RenderPipeline::createRenderPass(RenderTarget &target, std::vector<LoadStoreInfo> &loadStoreOps)
{
    //    assert(subpasses.size() > 0 && "Cannot create a render pass without any subpass");
    assert(!subPasses.empty());
    std::vector<SubpassInfo> subpassInfos(subPasses.size());
    auto subpassInfoIt = subpassInfos.begin();
    for (auto &subpass : subPasses)
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

    renderPass = std::make_unique<RenderPass>(device, target.getAttachments(), loadStoreOps, subpassInfos);
}

RenderPipeline::RenderPipeline(Device &device) : device(device) {}
