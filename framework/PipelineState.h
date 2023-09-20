#pragma once
#include "Shader.h"
#include "Vulkan.h"

class RenderPass;

struct PipelineLayout
{
    // The shader modules that this pipeline layout uses
    std::vector<Shader*> shader_modules;

    // The shader resources that this pipeline layout uses, indexed by their name
    std::unordered_map<std::string, ShaderResource*> shader_resources;
};

struct VertexInputState
{
    std::vector<VkVertexInputBindingDescription> bindings;

    std::vector<VkVertexInputAttributeDescription> attributes;
};

struct SpecializationConstantState
{
    VkBool32 depth_clamp_enable{VK_FALSE};

    VkBool32 rasterizer_discard_enable{VK_FALSE};

    VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};

    VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};

    VkFrontFace front_face{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    VkBool32 depth_bias_enable{VK_FALSE};
};

struct InputAssemblyState
{
    VkPrimitiveTopology topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    VkBool32 primitive_restart_enable{VK_FALSE};
};

struct RasterizationState
{
    VkBool32 depth_clamp_enable{VK_FALSE};

    VkBool32 rasterizer_discard_enable{VK_FALSE};

    VkPolygonMode polygon_mode{VK_POLYGON_MODE_FILL};

    VkCullModeFlags cull_mode{VK_CULL_MODE_BACK_BIT};

    VkFrontFace front_face{VK_FRONT_FACE_COUNTER_CLOCKWISE};

    VkBool32 depth_bias_enable{VK_FALSE};
};

struct ViewportState
{
    uint32_t viewport_count{1};

    uint32_t scissor_count{1};
};

struct MultisampleState
{
    VkSampleCountFlagBits rasterization_samples{VK_SAMPLE_COUNT_1_BIT};

    VkBool32 sample_shading_enable{VK_FALSE};

    float min_sample_shading{0.0f};

    VkSampleMask sample_mask{0};

    VkBool32 alpha_to_coverage_enable{VK_FALSE};

    VkBool32 alpha_to_one_enable{VK_FALSE};
};

struct DepthStencilState
{
    VkStencilOp fail_op{VK_STENCIL_OP_REPLACE};

    VkStencilOp pass_op{VK_STENCIL_OP_REPLACE};

    VkStencilOp depth_fail_op{VK_STENCIL_OP_REPLACE};

    VkCompareOp compare_op{VK_COMPARE_OP_NEVER};
};


struct ColorBlendAttachmentState
{
    VkBool32 blend_enable{VK_FALSE};

    VkBlendFactor src_color_blend_factor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dst_color_blend_factor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp color_blend_op{VK_BLEND_OP_ADD};

    VkBlendFactor src_alpha_blend_factor{VK_BLEND_FACTOR_ONE};

    VkBlendFactor dst_alpha_blend_factor{VK_BLEND_FACTOR_ZERO};

    VkBlendOp alpha_blend_op{VK_BLEND_OP_ADD};

    VkColorComponentFlags color_write_mask{
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
};


struct ColorBlendState
{
    VkBool32 logic_op_enable{VK_FALSE};

    VkLogicOp logic_op{VK_LOGIC_OP_CLEAR};

    std::vector<ColorBlendAttachmentState> attachments;
};


struct PipelineState
{
    PipelineLayout* pipeline_layout{nullptr};

    const RenderPass* render_pass{nullptr};

    SpecializationConstantState specializationConstantState{};

    VertexInputState vertexInputState{};

    InputAssemblyState inputAssemblyState{};

    RasterizationState rasterizationState{};

    ViewportState viewportState{};

    MultisampleState multisampleState{};

    DepthStencilState depthStencilState{};

    ColorBlendState colorBlendState{};
};
