#pragma once


#include "Core/Vulkan.h"
#include "RenderTarget.h"
#include "Core/CommandBuffer.h"

#include "Core/Shader/Shader.h"
#include "PipelineLayout.h"


class Device;

class Subpass;

class RenderPass;

class FrameBuffer;


class Pipeline {
public:
    VkPipeline getHandle() const { return pipeline; }

    Pipeline(Device &device, const PipelineState &pipelineState);

protected:
    VkPipeline pipeline;
    Device &device;
};

