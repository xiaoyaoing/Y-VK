#include "Pipeline.h"
#include "Core/CommandBuffer.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "Core/Device/Device.h"
#include <array>


Pipeline::Pipeline(Device& device, const PipelineState& pipelineState) : device(device)
{
    auto type = pipelineState.getPipelineType();
    auto &  shaders = pipelineState.getPipelineLayout().getShaders();
	std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;

	std::ranges::transform(shaders.begin(), shaders.end(), std::back_inserter(stageCreateInfos), [](const Shader& shader)
	{
		return shader.PipelineShaderStageCreateInfo();
	});

    if(type == PIPELINE_TYPE::E_GRAPHICS)
    {
        VkGraphicsPipelineCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};


       

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
        VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.getHandle(), nullptr, 1, &createInfo, nullptr, &pipeline))
    }
    else if(type == PIPELINE_TYPE::E_RAY_TRACING)
    {
		auto settings = pipelineState.getRtPassSettings();

    	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
    	
        int stage_idx = 0;
		for (const auto& shader : shaders) {
		VkRayTracingShaderGroupCreateInfoKHR group{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
			
		switch (shader.getStage()) {
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
			case VK_SHADER_STAGE_MISS_BIT_KHR: {
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				group.generalShader = stage_idx;
				break;
			}
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: {
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				group.closestHitShader = stage_idx;
				break;
			}
			case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: {
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				group.anyHitShader = stage_idx;
				break;
			}
			case VK_SHADER_STAGE_VERTEX_BIT:
			case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			case VK_SHADER_STAGE_GEOMETRY_BIT:
			case VK_SHADER_STAGE_FRAGMENT_BIT:
			case VK_SHADER_STAGE_COMPUTE_BIT:
			case VK_SHADER_STAGE_ALL_GRAPHICS:
			case VK_SHADER_STAGE_ALL:
			case VK_SHADER_STAGE_INTERSECTION_BIT_KHR:
			case VK_SHADER_STAGE_CALLABLE_BIT_KHR:
			case VK_SHADER_STAGE_TASK_BIT_NV:
			case VK_SHADER_STAGE_MESH_BIT_NV:
			case VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI:
				break;
		}
		groups.push_back(group);
		stage_idx++;
	}

	// VkSpecializationInfo specialization_info = {};
	// if (!settings.specialization_data.empty()) {
	// 	specialization_info.dataSize = settings.specialization_data.size() * sizeof(uint32_t);
	// 	specialization_info.mapEntryCount = (uint32_t)settings.specialization_data.size();
	// 	specialization_info.pMapEntries = entries.data();
	// 	specialization_info.pData = settings.specialization_data.data();
	// 	for (auto& stage : stages) {
	// 		stage.pSpecializationInfo = &specialization_info;
	// 	}
	// }
	VkRayTracingPipelineCreateInfoKHR pipeline_CI = {VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};

    	
	pipeline_CI.stageCount = stageCreateInfos.size();
	pipeline_CI.pStages = stageCreateInfos.data();
	pipeline_CI.groupCount = static_cast<uint32_t>(groups.size());
	pipeline_CI.pGroups = groups.data();
	pipeline_CI.maxPipelineRayRecursionDepth = settings.maxDepth;
	pipeline_CI.layout  = pipelineState.getPipelineLayout().getHandle();
	pipeline_CI.flags = 0;
	VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(device.getHandle(), {}, {}, 1, &pipeline_CI, nullptr, &pipeline));
    }

    else if(type == PIPELINE_TYPE::E_COMPUTE)
    {
        
    }
}
