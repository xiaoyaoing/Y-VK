#pragma once

#include "Core/Vulkan.h"

#include "PipelineLayout.h"

class RenderPass;

struct VertexInputState {
    std::vector<VkVertexInputBindingDescription> bindings;

    std::vector<VkVertexInputAttributeDescription> attributes;
};

class SpecializationConstantState {
public:
    void reset();

    bool isDirty() const;

    void clearDirty();

    template<class T>
    void setConstant(uint32_t constantId, const T& data) {
        setConstant(constantId, toUint32(static_cast<std::uint32_t>(data)));
    }

    void setConstant(uint32_t constantId, const std::vector<uint8_t>& data);

    void setSpecializationConstantState(const std::map<uint32_t, std::vector<uint8_t>>& state);

    const std::map<uint32_t, std::vector<uint8_t>>& getSpecializationConstantState() const;

private:
    bool dirty{false};
    // Map tracking state of the Specialization Constants
    std::map<uint32_t, std::vector<uint8_t>> specializationConstantState;
};

struct InputAssemblyState {
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    VkBool32 primitiveRestartEnable{VK_FALSE};
};

struct RasterizationState {
    VkBool32 depthClampEnable{VK_FALSE};

    VkBool32 rasterizerDiscardEnable{VK_FALSE};

    VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};

    VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};

    VkFrontFace frontFace{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    VkBool32 depthBiasEnable{VK_FALSE};

    //For ext
    const void* pNext{nullptr};
};

struct ViewportState {
    uint32_t viewportCount{1};

    uint32_t scissorCount{1};
};

struct MultisampleState {
    VkSampleCountFlagBits rasterizationSamples{VK_SAMPLE_COUNT_1_BIT};

    VkBool32 sampleShadingEnable{VK_FALSE};

    float minSampleShading{0.0f};

    VkSampleMask sampleMask{0};

    VkBool32 alphaToCoverageEnable{VK_FALSE};

    VkBool32 alphaToOneEnable{VK_FALSE};
};

struct StencilOpState {
    VkStencilOp failOp{VK_STENCIL_OP_REPLACE};

    VkStencilOp passOp{VK_STENCIL_OP_REPLACE};

    VkStencilOp depthFailOp{VK_STENCIL_OP_REPLACE};

    VkCompareOp compareOp{VK_COMPARE_OP_LESS_OR_EQUAL};
};

struct DepthStencilState {
    VkBool32 depthTestEnable{VK_TRUE};

    VkBool32 depthWriteEnable{VK_TRUE};

    // Note: Using reversed depth-buffer for increased precision, so Greater depth values are kept
    VkCompareOp depthCompareOp{VK_COMPARE_OP_GREATER_OR_EQUAL};

    VkBool32 depthBoundsTestEnable{VK_FALSE};

    VkBool32 stencilTestEnable{VK_FALSE};

    StencilOpState front{};

    StencilOpState back{};
};

struct ColorBlendAttachmentState {
    VkBool32 blendEnable{VK_FALSE};

    VkBlendFactor srcColorBlendFactor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dstColorBlendFactor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp colorBlendOp{VK_BLEND_OP_ADD};

    VkBlendFactor srcAlphaBlendFactor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dstAlphaBlendFactor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp alphaBlendOp{VK_BLEND_OP_ADD};

    VkColorComponentFlags colorWriteMask{
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
};

struct RTPipelineSettings {
    std::vector<Shader>        shaders;
    VkAccelerationStructureKHR accel;
    uint32_t                   maxDepth;
    VkExtent3D                 dims;
};

struct ColorBlendState {
    VkBool32 logicOpEnable{VK_FALSE};

    VkLogicOp logicOp{VK_LOGIC_OP_CLEAR};

    std::vector<ColorBlendAttachmentState> attachments;
};

enum class PIPELINE_TYPE : uint8_t {
    E_GRAPHICS,
    E_COMPUTE,
    E_RAY_TRACING
};

struct PipelineState {
public:
    void reset();

    PipelineState& setPipelineLayout(PipelineLayout& pipelineLayout);

    PipelineState& setRenderPass(const RenderPass& renderPass);

    PipelineState& setSpecializationConstant(uint32_t constantId, const std::vector<uint8_t>& data);

    PipelineState& setVertexInputState(const VertexInputState& vertexInputState);

    PipelineState& setInputAssemblyState(const InputAssemblyState& inputAssemblyState);

    PipelineState& setRasterizationState(const RasterizationState& rasterizationState);

    PipelineState& setViewportState(const ViewportState& viewportState);

    PipelineState& setMultisampleState(const MultisampleState& multisampleState);

    PipelineState& setDepthStencilState(const DepthStencilState& depthStencilState);

    PipelineState& setColorBlendState(const ColorBlendState& colorBlendState);

    PipelineState& setSubpassIndex(uint32_t subpassIndex);

    PipelineState& setPipelineType(PIPELINE_TYPE pipelineType);

    PipelineState& enableConservativeRasterization(VkPhysicalDevice device);

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

    PIPELINE_TYPE getPipelineType() const;

    bool isDirty() const;

    PipelineState& clearDirty();

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

public:
    const RTPipelineSettings& getrTPipelineSettings() const;
    PipelineState&            setrTPipelineSettings(const RTPipelineSettings& rTPipelineSettings);

private:
    uint32_t subpassIndex{0U};

    RTPipelineSettings rTPipelineSettings{};

    PIPELINE_TYPE pipelineType{PIPELINE_TYPE::E_GRAPHICS};

    bool dirty{false};
};