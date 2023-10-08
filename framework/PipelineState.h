#pragma once
#include "Vulkan.h"

#include <PipelineLayout.h>

class RenderPass;


struct VertexInputState
{
    std::vector<VkVertexInputBindingDescription> bindings;

    std::vector<VkVertexInputAttributeDescription> attributes;
};

struct SpecializationConstantState
{
    VkBool32 depthClampEnable{VK_FALSE};

    VkBool32 rasterizerDiscardEnable{VK_FALSE};

    VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};

    VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};

    VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    VkBool32 depthBiasEnable{VK_FALSE};
};

struct InputAssemblyState
{
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    VkBool32 primitiveRestartEnable{VK_FALSE};
};

struct RasterizationState
{
    VkBool32 depthClampEnable{VK_FALSE};

    VkBool32 rasterizerDiscardEnable{VK_FALSE};

    VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};

    VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};

    VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    VkBool32 depthBiasEnable{VK_FALSE};
};

struct ViewportState
{
    uint32_t viewportCount{1};

    uint32_t scissorCount{1};
};

struct MultisampleState
{
    VkSampleCountFlagBits rasterizationSamples{VK_SAMPLE_COUNT_1_BIT};

    VkBool32 sampleShadingEnable{VK_FALSE};

    float minSampleShading{0.0f};

    VkSampleMask sampleMask{0};

    VkBool32 alphaToCoverageEnable{VK_FALSE};

    VkBool32 alphaToOneEnable{VK_FALSE};
};

struct StencilOpState
{
    VkStencilOp failOp{VK_STENCIL_OP_REPLACE};

    VkStencilOp passOp{VK_STENCIL_OP_REPLACE};

    VkStencilOp depthFailOp{VK_STENCIL_OP_REPLACE};

    VkCompareOp compareOp{VK_COMPARE_OP_NEVER};
};

struct DepthStencilState
{
    VkBool32 depthTestEnable{VK_TRUE};

    VkBool32 depthWriteEnable{VK_TRUE};

    // Note: Using reversed depth-buffer for increased precision, so Greater depth values are kept
    VkCompareOp depthCompareOp{VK_COMPARE_OP_GREATER};

    VkBool32 depthBoundsTestEnable{VK_FALSE};

    VkBool32 stencilTestEnable{VK_FALSE};

    StencilOpState front{};

    StencilOpState back{};
};

struct ColorBlendAttachmentState
{
    VkBool32 blendEnable{VK_FALSE};

    VkBlendFactor srcColorBlendFactor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dstColorBlendFactor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp colorBlendOp{VK_BLEND_OP_ADD};

    VkBlendFactor srcAlphaBlendFactor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dstAlphaBlendFactor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp alphaBlendOp{VK_BLEND_OP_ADD};

    VkColorComponentFlags colorWriteMask{
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
};

struct ColorBlendState
{
    VkBool32 logicOpEnable{VK_FALSE};

    VkLogicOp logicOp{VK_LOGIC_OP_CLEAR};

    std::vector<ColorBlendAttachmentState> attachments;
};

struct PipelineState
{
public:
    void reset();

    void setPipelineLayout(PipelineLayout& pipelineLayout);

    void setRenderPass(const RenderPass& renderPass);

    void setSpecializationConstant(uint32_t constantId, const std::vector<uint8_t>& data);

    void setVertexInputState(const VertexInputState& vertexInputState);

    void setInputAssemblyState(const InputAssemblyState& inputAssemblyState);

    void setRasterizationState(const RasterizationState& rasterizationState);

    void setViewportState(const ViewportState& viewportState);

    void setMultisampleState(const MultisampleState& multisampleState);

    void setDepthStencilState(const DepthStencilState& depthStencilState);

    void setColorBlendState(const ColorBlendState& colorBlendState);

    void setSubpassIndex(uint32_t subpassIndex);

    const PipelineLayout& getPipelineLayout() const;

    const RenderPass* getRenderPass() const;

    const SpecializationConstantState& getSpecializationConstantState() const;

    const VertexInputState& getVertexInputState() const;

    const InputAssemblyState& getInputAssemblyState() const;

    const RasterizationState& getRasterizationState() const;

    const ViewportState& getViewportState() const;

    const MultisampleState& getMultisampleState() const;

    const DepthStencilState& getDepthStencilState() const;

    const ColorBlendState& getColorBlendState() const;

    uint32_t getSubpassIndex() const;

    bool isDirty() const;

    void clearDirty();

private:
    PipelineLayout* pipelineLayout{nullptr};

    const RenderPass* renderPass{nullptr};

    SpecializationConstantState specializationConstantState{};

    VertexInputState vertexInputState{};

    InputAssemblyState inputAssemblyState{};

    RasterizationState rasterizationState{};

    ViewportState viewportState{};

    MultisampleState multisampleState{};

    DepthStencilState depthStencilState{};

    ColorBlendState colorBlendState{};

    uint32_t subpassIndex{0U};

    bool dirty{false};
};
