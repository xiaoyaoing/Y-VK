#include "PassNode.h"

#include "Device.h"
#include <Common/ResourceCache.h>

#include "RenderContext.h"
#include "VirtualResource.h"
#include "RenderGraph/RenderGraph.h"
#include "VertexData.h"
#include "Images/ImageUtil.h"


void RenderPassNode::RenderPassData::devirtualize(RenderGraph& renderGraph, const RenderPassNode& node)
{
    std::vector<sg::SgImage*> images;
    std::vector<Attachment> attachments;
    for (const auto& color : desc.color)
    {
        auto texture = renderGraph.getResource(color);
        auto hwTexture = texture->getHwTexture();

        auto undefined = texture->first == &node && !texture->imported;
        auto write = renderGraph.isWrite(color, &node);
        auto attachment = Attachment{
            .format = hwTexture->getFormat(),
            .samples = hwTexture->getVkImage().getSampleCount(),
            .usage = hwTexture->getVkImage().getUseFlags(),
            .initial_layout = ImageUtil::getVkImageLayout(
                hwTexture->getVkImage().getLayout(hwTexture->getVkImageView().getSubResourceRange())),
            .loadOp = (undefined | write) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE
        };

        images.push_back(hwTexture);
        attachments.push_back(attachment);
    }
    renderTarget = std::make_unique<RenderTarget>(images, attachments);
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
    renderTargetData.devirtualize(renderGraph, *this);
    // resolveTextureUsages(renderGraph, commandBuffer);

    auto& renderTarget = renderTargetData.getRenderTarget();


    auto hwTextures = renderTarget.getHwTextures();

    // //  for (auto &hwTexture: renderTarget.getHwTextures()) {
    // {
    //     // Image 0 is the swapchain
    //     ImageMemoryBarrier memoryBarrier{};
    //     memoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     memoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //     memoryBarrier.srcAccessMask = 0;
    //     memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    //     memoryBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //     memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    //
    //     ImageMemoryBarrier depthMemoryBarrier{};
    //     depthMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //     depthMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //     depthMemoryBarrier.srcAccessMask = 0;
    //     depthMemoryBarrier.dstAccessMask =
    //         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    //     depthMemoryBarrier.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //     depthMemoryBarrier.dstStageMask =
    //         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    //
    //
    //     // commandBuffer.imageMemoryBarrier(hwTextures[0]->getVkImageView(), memoryBarrier);
    //
    //     // Skip 1 as it is handled later as a depth-stencil attachment
    //     for (size_t i = 0; i < renderTarget.getHwTextures().size(); ++i)
    //     {
    //         if (isDepthOrStencilFormat(hwTextures[i]->getFormat()))
    //         {
    //             commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), depthMemoryBarrier);
    //         }
    //         else
    //         {
    //             if (i > 1)
    //             {
    //                 memoryBarrier.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    //                 memoryBarrier.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    //             }
    //             commandBuffer.imageMemoryBarrier(hwTextures[i]->getVkImageView(), memoryBarrier);
    //         }
    //     }
    // }

    std::vector<SubpassInfo> subpasses{};

    // if(renderTarget.getAttachments().size() == 3)
    // {
    subpasses.push_back(SubpassInfo{.outputAttachments = {2, 3, 4}});
    subpasses.push_back(SubpassInfo{.inputAttachments = {2, 3, 4}, .outputAttachments = {0}});
    // }


    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = RenderContext::g_context->getSwapChainFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentDescription depthAttachment{};

    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;
    subpass.pResolveAttachments = nullptr;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
    // std::vector<VkSubpassDescription> subpasses = {subpass};

    VkRenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_create_info.pAttachments = attachments.data();
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 0;
    render_pass_create_info.pDependencies = dependencies.data();

    VkRenderPass vkRenderPass;
    VK_CHECK_RESULT(
        vkCreateRenderPass(renderGraph.getDevice().getHandle(), &render_pass_create_info, nullptr, &vkRenderPass));


    //   auto& renderPass = renderGraph.getDevice().getResourceCache().requestRenderPass(
    //       renderTarget.getAttachments(), subpasses);
    //
    //   // RenderPass pass(renderGraph.getDevice(), vkRenderPass);
    //   // //   commandBuffer.beginRenderPass()
    //
    // //renderPass.setHandle(vkRenderPass);
    //
    //   auto& framebuffer = renderGraph.getDevice().getResourceCache().requestFrameBuffer(
    //       renderTargetData.getRenderTarget(), renderPass);


    //
    // std::vector<VkClearValue> clearValues = {};
    //
    // commandBuffer.beginRenderPass(renderPass, framebuffer, renderTarget.getDefaultClearValues(), {});
    //
    RenderPassContext context = {
        // .renderPass = renderPass, .commandBuffer = commandBuffer,
        .renderTarget = renderTarget,
        // .pipeline = getPipeline(renderGraph.getDevice(), renderPass),
    };
    //
    // auto& renderContext = *RenderContext::g_context;
    //
    // ColorBlendState colorBlendState = renderContext.getPipelineState().getColorBlendState();
    // colorBlendState.attachments.resize(renderPass.getColorOutputCount(0));
    // renderContext.getPipelineState().setColorBlendState(colorBlendState);
    mRenderPass->execute(context);
}

void RenderPassNode::declareRenderTarget(const char* name, const RenderGraphPassDescriptor& descriptor)
{
    renderTargetData.name = name;

    renderTargetData.desc = descriptor;
}

RenderPassNode::RenderPassNode(RenderGraph& renderGraph, const char* name, RenderGraphPassBase* base) : mRenderPass(
    base), name(name)
{
}

void RenderPassNode::declareRenderPass(const char* name, const RenderGraphPassDescriptor& descriptor)
{
    renderTargetData.desc = descriptor;
}

// void PassNode::addTextureUsage(RenderGraphHandle id, RenderGraphTexture::Usage usage)
// {
//     
// }

void PassNode::resolveTextureUsages(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
    for (auto& textureIt : textureUsages)
    {
        const auto* pResource = textureIt.first;
        const auto newLayout = ImageUtil::getDefaultLayout(textureIt.second);
        const auto subsubsource = pResource->getHwTexture()->getVkImageView().getSubResourceRange();
        pResource->getHwTexture()->getVkImage().transitionLayout(commandBuffer, newLayout, subsubsource);
    }
}

void PassNode::addTextureUsage(const RenderGraphTexture* texture, RenderGraphTexture::Usage usage)
{
    if (textureUsages.contains(texture))
        textureUsages[texture] = textureUsages[texture] | usage;
    else
        textureUsages[texture] = usage;
}

void PresentPassNode::execute(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
}
