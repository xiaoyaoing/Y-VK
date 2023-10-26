#include "PassNode.h"

#include "Device.h"
#include <Common/ResourceCache.h>

#include "RenderContext.h"
#include "VirtualResource.h"
#include "RenderGraph/RenderGraph.h"
#include "VertexData.h"


void RenderPassNode::RenderPassData::devirtualize(RenderGraph& renderGraph)
{
    std::vector<Image> images;
    std::vector<sg::SgImage*> attachments;
    for (size_t i = 0; i < desc.color.size(); i++)
    {
        const auto* pResource = static_cast<const Resource<RenderGraphTexture>*>(renderGraph.
            getResource(desc.color[i]));
        attachments.push_back(pResource->getHandle().getHwTexture());
        //images.emplace_back(hwTexture->image);
    }
    renderTarget = std::make_unique<RenderTarget>(attachments);
}

RenderTarget& RenderPassNode::RenderPassData::getRenderTarget()
{
    return *renderTarget;
}

VkPipeline pipeline = VK_NULL_HANDLE;

VkPipeline getPipeline(Device& device, RenderPass& renderPass)
{
    if (pipeline != VK_NULL_HANDLE)
        return pipeline;
    std::vector<Shader> shaders;
    shaders.emplace_back(device, FileUtils::getShaderPath() + "base.vert");
    shaders.emplace_back(device, FileUtils::getShaderPath() + "base.frag");


    auto descriptorLayout = std::make_unique<DescriptorLayout>(device);
    descriptorLayout->addBinding(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
    descriptorLayout->createLayout(0);

    // VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    // pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    // pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    //
    // pipelineLayoutCreateInfo.setLayoutCount = 1;
    // auto &t = device.getResourceCache().requestDescriptorLayout(shaders);
    // VkDescriptorSetLayout descriptorSetLayouts[] = {t.getHandle()};
    // pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    //
    // VkPipelineLayout pipelineLayout;
    // if (vkCreatePipelineLayout(device.getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) !=
    //     VK_SUCCESS)
    //     RUN_TIME_ERROR("Failed to create pipeline layout");


    if (pipeline != VK_NULL_HANDLE)
        return pipeline;


    auto& pipelineLayOut = device.getResourceCache().requestPipelineLayout(shaders);
    RenderContext::g_context->getPipelineState().setPipelineLayout(pipelineLayOut);

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        shaders[0].PipelineShaderStageCreateInfo(),
        shaders[1].PipelineShaderStageCreateInfo()
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 1024;
    viewport.height = 1024;
    viewport.minDepth = 0;
    viewport.maxDepth = 1.0;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = {1024, 1024};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.pNext = nullptr;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.pNext = nullptr;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0;
    depthStencil.maxDepthBounds = 1.0;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStateEnables.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    // pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr;
    //  pipelineInfo.layout = pipelineLayout;
    pipelineInfo.layout = pipelineLayOut.getHandle();

    pipelineInfo.renderPass = renderPass.getHandle();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VK_CHECK_RESULT(
        vkCreateGraphicsPipelines(device.getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline))
    return pipeline;
}

void RenderPassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
    renderTargetData.devirtualize(renderGraph);

    auto& renderTarget = renderTargetData.getRenderTarget();
    auto hwTextures = renderTarget.getHwTextures();

    //  for (auto &hwTexture: renderTarget.getHwTextures()) {
    {
        // Image 0 is the swapchain
        ImageMemoryBarrier memoryBarrier{};
        memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        memoryBarrier.srcAccessMask = 0;
        memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        ImageMemoryBarrier depthMemoryBarrier{};
        depthMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthMemoryBarrier.srcAccessMask = 0;
        depthMemoryBarrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        depthMemoryBarrier.dstStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;


        // commandBuffer.imageMemoryBarrier(hwTextures[0]->getVkImageView(), memoryBarrier);

        // Skip 1 as it is handled later as a depth-stencil attachment
        for (size_t i = 0; i < renderTarget.getHwTextures().size(); ++i)
        {
            if (isDepthOrStencilFormat(hwTextures[i]->getFormat()))
            {
                commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), depthMemoryBarrier);
            }
            else
                commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), memoryBarrier);
        }
    }
    //
    // {
    //     ImageMemoryBarrier memoryBarrier{};
    //     memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     memoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     memoryBarrier.srcAccessMask = 0;
    //     memoryBarrier.dstAccessMask =
    //         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //     memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //     memoryBarrier.dstStageMask =
    //         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //
    //     commandBuffer.imageMemoryBarrier(hwTextures[1]->getVkImageView(), memoryBarrier);
    // }

    //setImageLayout(commandBuffer.getHandle(),hwTexture->getVkImage().getHandle(),)
    //}

    auto& renderPass = renderGraph.getDevice().getResourceCache().requestRenderPass(renderTarget.getAttachments(), {});

    auto& framebuffer = renderGraph.getDevice().getResourceCache().requestFrameBuffer(
        renderTargetData.getRenderTarget(), renderPass);

    std::vector<VkClearValue> clearValues = {};

    commandBuffer.beginRenderPass(renderPass, framebuffer, renderTarget.getDefaultClearValues(), {});

    RenderPassContext context = {
        .renderPass = renderPass, .commandBuffer = commandBuffer,
        .renderTarget = renderTarget,
        // .pipeline = getPipeline(renderGraph.getDevice(), renderPass),
    };

    auto& renderContext = *RenderContext::g_context;

    ColorBlendState colorBlendState = renderContext.getPipelineState().getColorBlendState();
    colorBlendState.attachments.resize(renderPass.getColorOutputCount(0));
    renderContext.getPipelineState().setColorBlendState(colorBlendState);
    mRenderPass->execute(context);
}

void RenderPassNode::declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor)
{
    renderTargetData.name = name;

    renderTargetData.desc = descriptor;
}

RenderPassNode::RenderPassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base) : mRenderPass(
    base)
{
}

void PresentPassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
}
