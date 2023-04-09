#include "Vulkan.h"
class Device;
struct PipelineInfo{
    VkPipelineInputAssemblyStateCreateInfo      inputAssemblyState;
    VkPipelineRasterizationStateCreateInfo      rasterizationState;
    VkPipelineColorBlendAttachmentState         blendAttachmentStates[8];
    VkPipelineDepthStencilStateCreateInfo       depthStencilState;
    VkPipelineMultisampleStateCreateInfo        multisampleState;
    VkPipelineTessellationStateCreateInfo       tessellationState;

    VkShaderModule  vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule  fragShaderModule = VK_NULL_HANDLE;
    VkShaderModule  compShaderModule = VK_NULL_HANDLE;
    VkShaderModule  tescShaderModule = VK_NULL_HANDLE;
    VkShaderModule  teseShaderModule = VK_NULL_HANDLE;
    VkShaderModule  geomShaderModule = VK_NULL_HANDLE;

    uint32_t          subpass = 0;
    uint32_t          colorAttachmentCount = 1;

    PipelineInfo(){
        inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        rasterizationState.sType  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState.depthClampEnable        = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.depthBiasEnable         = VK_FALSE;
        rasterizationState.lineWidth               = 1.0f;

        for (uint32_t i = 0; i < 8; ++i)
        {
            blendAttachmentStates[i] = {};
            blendAttachmentStates[i].colorWriteMask = (
                    VK_COLOR_COMPONENT_R_BIT |
                    VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT
            );
            blendAttachmentStates[i].blendEnable = VK_FALSE;
            blendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachmentStates[i].colorBlendOp        = VK_BLEND_OP_ADD;
            blendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blendAttachmentStates[i].alphaBlendOp        = VK_BLEND_OP_ADD;
        }
        depthStencilState.sType =  VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilState.depthTestEnable       = VK_TRUE;
        depthStencilState.depthWriteEnable      = VK_TRUE;
        depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
        depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
        depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
        depthStencilState.stencilTestEnable     = VK_TRUE;
        depthStencilState.front                 = depthStencilState.back;

        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.pSampleMask          = nullptr;

        tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState.patchControlPoints = 0;
    }

    void fillShaderStages(std::vector<VkPipelineShaderStageCreateInfo> & shaderStages) const {
        if (vertShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStageCreateInfo.module = vertShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (fragShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStageCreateInfo.module = fragShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (compShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
            shaderStageCreateInfo.module = compShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (geomShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
            shaderStageCreateInfo.module = geomShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (tescShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
            shaderStageCreateInfo.module = tescShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }

        if (teseShaderModule != VK_NULL_HANDLE)
        {
            VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
            shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
            shaderStageCreateInfo.module = teseShaderModule;
            shaderStageCreateInfo.pName  = "main";
            shaderStages.push_back(shaderStageCreateInfo);
        }
    }

};


class Pipeline{
public:
    inline VkPipeline getHandle(){return _pipeline;}
    Pipeline(const PipelineInfo & info,ptr<Device>  device,const std::vector<VkVertexInputBindingDescription>& inputBindings,
             const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributs,
             VkPipelineLayout pipelineLayout,
             VkRenderPass renderPass);
    explicit Pipeline(const VkPipeline pipeline) : _pipeline(pipeline) {}
    void cleanup(){}
protected:
    VkPipeline  _pipeline;
    ptr<Device> _device;
    VkPipelineLayout  _layout;
};