//
// Created by pc on 2023/8/23.
//

#include "Gui.h"
#include <Common/VkCommon.h>
#include <Shader.h>
#include "Device.h"
#include "App/Application.h"
#include "Common/ResourceCache.h"

Gui::Gui(Device& device) : device(device)
{
    ImGui::CreateContext();
    // Color scheme
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    // Dimensions
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = scale;

    std::vector<Shader> shaders{};
    shaders.emplace_back(Shader(device, FileUtils::getShaderPath("gui.vert")));
    shaders.emplace_back(Shader(device, FileUtils::getShaderPath("gui.frag")));
    pipelineLayout = &device.getResourceCache().requestPipelineLayout(shaders);
}

void Gui::prepare(const VkPipelineCache pipelineCache, const VkRenderPass renderPass)
{
    // descriptorLayout = std::make_unique<DescriptorLayout>(device);
    // descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
    // descriptorLayout->createLayout(0);
    //
    //
    // Shader vertexShader(device, FileUtils::getShaderPath() + "gui.vert"), fragShader(device,
    //                                                                                  FileUtils::getShaderPath() +
    //                                                                                  "gui.frag");
    // shaders = {vertexShader.PipelineShaderStageCreateInfo(),
    //            fragShader.PipelineShaderStageCreateInfo()};
    //
    // auto pushConstantRange = vkCommon::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT,
    //                                                                    sizeof(PushConstBlock), 0);
    //
    // std::vector<VkDescriptorSetLayout> layout{descriptorLayout->getHandle()};
    // auto pipelineLayoutCreateInfo = vkCommon::initializers::pipelineLayoutCreateInfo(layout);
    //
    // pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    // pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    // VK_CHECK_RESULT(vkCreatePipelineLayout(device.getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout))
    //
    // VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
    //         vkCommon::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0,
    //                                                                      VK_FALSE);
    //
    // VkPipelineRasterizationStateCreateInfo rasterizationState =
    //         vkCommon::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
    //                                                                      VK_FRONT_FACE_COUNTER_CLOCKWISE);
    //
    // // Enable blending
    // VkPipelineColorBlendAttachmentState blendAttachmentState{};
    // blendAttachmentState.blendEnable = VK_TRUE;
    // blendAttachmentState.colorWriteMask =
    //         VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    // blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    // blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    // blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    // blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    // blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    //
    // VkPipelineColorBlendStateCreateInfo colorBlendState =
    //         vkCommon::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    //
    // VkPipelineDepthStencilStateCreateInfo depthStencilState =
    //         vkCommon::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);
    //
    // VkPipelineViewportStateCreateInfo viewportState =
    //         vkCommon::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
    //
    // VkPipelineMultisampleStateCreateInfo multisampleState =
    //         vkCommon::initializers::pipelineMultisampleStateCreateInfo(rasterSamples);
    //
    // std::vector<VkDynamicState> dynamicStateEnables = {
    //         VK_DYNAMIC_STATE_VIEWPORT,
    //         VK_DYNAMIC_STATE_SCISSOR};
    // VkPipelineDynamicStateCreateInfo dynamicState =
    //         vkCommon::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
    //
    // VkGraphicsPipelineCreateInfo pipelineCreateInfo = vkCommon::initializers::pipelineCreateInfo(pipelineLayout,
    //                                                                                              renderPass);
    //
    // pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    // pipelineCreateInfo.pRasterizationState = &rasterizationState;
    // pipelineCreateInfo.pColorBlendState = &colorBlendState;
    // pipelineCreateInfo.pMultisampleState = &multisampleState;
    // pipelineCreateInfo.pViewportState = &viewportState;
    // pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    // pipelineCreateInfo.pDynamicState = &dynamicState;
    // pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaders.size());
    // pipelineCreateInfo.pStages = shaders.data();
    // pipelineCreateInfo.subpass = subPass;
    //
    // // Vertex bindings an attributes based on ImGui vertex definition
    // std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
    //         vkCommon::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    // };
    // std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
    //         vkCommon::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert,
    //                                                                                                         pos)), // Location 0: Position
    //         vkCommon::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT,
    //                                                                 offsetof(ImDrawVert, uv)), // Location 1: UV
    //         vkCommon::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM,
    //                                                                 offsetof(ImDrawVert, col)), // Location 0: Color
    // };
    // VkPipelineVertexInputStateCreateInfo vertexInputState = vkCommon::initializers::pipelineVertexInputStateCreateInfo();
    // vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
    // vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
    // vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
    // vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();
    //
    // pipelineCreateInfo.pVertexInputState = &vertexInputState;
    //
    // VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.getHandle(), pipelineCache, 1, &pipelineCreateInfo, nullptr,
    //                                           &pipeline));
}

void Gui::prepareResoucrces(Application* app)
{
    fontTexture = Texture::loadTexture(device, FileUtils::getResourcePath() + "Roboto-Medium.ttf");


    vertexInputState.attributes = {
        vkCommon::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert,
                                                                    pos)), // Location 0: Position
        vkCommon::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT,
                                                                offsetof(ImDrawVert, uv)), // Location 1: UV
        vkCommon::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM,
                                                                offsetof(ImDrawVert, col)), // Location 0: Color
    };

    vertexInputState.bindings = {
        vkCommon::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
    };

    ColorBlendAttachmentState colorBlendAttachmentState;


    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlendState.attachments = {colorBlendAttachmentState};

    // Descriptor pool
    // std::vector<VkDescriptorPoolSize> poolSizes = {
    //     vkCommon::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
    // };

    // descriptorPool = std::make_unique<DescriptorPool>(device, poolSizes, 2);
    //
    //
    // descriptorSet = std::make_unique<DescriptorSet>(device, *descriptorPool, *descriptorLayout, 1);
    //
    // descriptorSet->updateImage({vkCommon::initializers::descriptorImageInfo(fontTexture)}, 0,
    //                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
}

bool Gui::update()
{
    ImDrawData* imDrawData = ImGui::GetDrawData();

    if (!imDrawData)
    {
        return false;
    };

    // Note: Alignment is done inside buffer creation
    VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
    VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

    if ((vertexBufferSize == 0) || (indexBufferSize == 0))
    {
        return false;
    }

    if (!vertexBuffer || !indexBuffer || vertexBuffer->getSize() != vertexBufferSize ||
        indexBuffer->getSize() != indexBufferSize)
    {
        if (!vertexBuffer || vertexBuffer->getSize() != vertexBufferSize)
            vertexBuffer = std::make_unique<Buffer>(device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
        if (!indexBuffer || indexBuffer->getSize() != indexBufferSize)
            indexBuffer = std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                   VMA_MEMORY_USAGE_CPU_TO_GPU);
        uint64_t vtxOffset = 0, idxOffset = 0;
        for (int n = 0; n < imDrawData->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = imDrawData->CmdLists[n];
            vertexBuffer->uploadData(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                                     vtxOffset);
            indexBuffer->uploadData(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), idxOffset);
            vtxOffset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
            idxOffset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
        }
        return true;
    }
    return false;
}

void Gui::draw(CommandBuffer& commandBuffer)
{
    auto& renderContext = *RenderContext::g_context;
    ImDrawData* imDrawData = ImGui::GetDrawData();
    int32_t vertexOffset = 0;
    int32_t indexOffset = 0;
    if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
    {
        return;
    }
    const ImGuiIO& io = ImGui::GetIO();


    renderContext.getPipelineState().setPipelineLayout(*pipelineLayout);
    renderContext.getPipelineState().setVertexInputState(vertexInputState);
    renderContext.getPipelineState().setColorBlendState(colorBlendState);
    renderContext.getPipelineState().setRasterizationState({.cullMode = VK_CULL_MODE_NONE});

    pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
    pushConstBlock.translate = glm::vec2(-1.0f);

    const auto blockPtr = reinterpret_cast<const uint8_t*>(&pushConstBlock);
    std::vector<uint8_t> pushConstants(blockPtr, blockPtr + sizeof(PushConstBlock));
    renderContext.pushConstants(pushConstants);

    renderContext.bindImage(0, fontTexture.getImage().getVkImageView(),
                            fontTexture.getSampler(), 0, 0);


    std::vector<const Buffer*> vertexBuffers = {vertexBuffer.get()};

    commandBuffer.bindVertexBuffer(vertexBuffers, {0});
    commandBuffer.bindIndicesBuffer(*indexBuffer, 0);


    for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = imDrawData->CmdLists[i];
        for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
            VkRect2D scissorRect;
            scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
            scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
            scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
            scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
            commandBuffer.setScissor(0, {scissorRect});
            renderContext.flushAndDrawIndexed(
                commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
            indexOffset += pcmd->ElemCount;
        }
        vertexOffset += cmd_list->VtxBuffer.Size;
    }
}

void Gui::addGuiPass(RenderGraph& graph, RenderContext& renderContext)
{
    struct GuiPassData
    {
        RenderGraphHandle output;
    };
    graph.addPass<GuiPassData>("Gui Pass",
                               [&graph](RenderGraph::Builder& builder, GuiPassData& data)
                               {
                                   data.output = graph.getBlackBoard()["output"];
                                   data.output = builder.writeTexture(data.output, TextureUsage::COLOR_ATTACHMENT);
                                   //    data.output = builder.readTexture(data.output, TextureUsage::COLOR_ATTACHMENT);

                                   std::vector<RenderGraphSubpassInfo> subpassInfos;
                                   subpassInfos.push_back(RenderGraphSubpassInfo{
                                       .outputAttachments = {data.output}
                                   });
                                   builder.declare("gui pass", {.textures = {data.output}, .subpasses = subpassInfos});
                               },
                               [&renderContext,this](GuiPassData& data, const RenderPassContext& context)
                               {
                                   ImDrawData* imDrawData = ImGui::GetDrawData();
                                   int32_t vertexOffset = 0;
                                   int32_t indexOffset = 0;
                                   if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
                                   {
                                       renderContext.endRenderPass(context.commandBuffer);

                                       return;
                                   }
                                   const ImGuiIO& io = ImGui::GetIO();


                                   renderContext.getPipelineState().setPipelineLayout(*pipelineLayout);
                                   renderContext.getPipelineState().setVertexInputState(vertexInputState);
                                   renderContext.getPipelineState().setColorBlendState(colorBlendState);
                                   renderContext.getPipelineState().setRasterizationState({
                                       .cullMode = VK_CULL_MODE_NONE
                                   });

                                   pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
                                   pushConstBlock.translate = glm::vec2(-1.0f);

                                   const auto blockPtr = reinterpret_cast<const uint8_t*>(&pushConstBlock);
                                   std::vector<uint8_t> pushConstants(blockPtr, blockPtr + sizeof(PushConstBlock));
                                   renderContext.pushConstants(pushConstants);

                                   renderContext.bindImage(0, fontTexture.getImage().getVkImageView(),
                                                           fontTexture.getSampler(), 0, 0);


                                   std::vector<const Buffer*> vertexBuffers = {vertexBuffer.get()};

                                   context.commandBuffer.bindVertexBuffer(vertexBuffers, {0});
                                   context.commandBuffer.bindIndicesBuffer(*indexBuffer, 0);


                                   for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
                                   {
                                       const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                                       for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
                                       {
                                           const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                                           VkRect2D scissorRect;
                                           scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
                                           scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
                                           scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
                                           scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
                                           context.commandBuffer.setScissor(0, {scissorRect});
                                           renderContext.flushAndDrawIndexed(
                                               context.commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
                                           indexOffset += pcmd->ElemCount;
                                       }
                                       vertexOffset += cmd_list->VtxBuffer.Size;
                                   }
                                   renderContext.endRenderPass(context.commandBuffer);
                               }
    );
}

bool Gui::checkBox(const char* caption, bool* value)
{
    bool res = ImGui::Checkbox(caption, value);
    if (res) { updated = true; };
    return res;
}

void Gui::text(const char* formatstr, ...)
{
    va_list args;
    va_start(args, formatstr);
    ImGui::TextV(formatstr, args);
    va_end(args);
}
