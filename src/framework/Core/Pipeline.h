#pragma once


#include "Core/Vulkan.h"
#include "RenderTarget.h"
#include "Core/CommandBuffer.h"

#include "Core/Shader/Shader.h"
#include "PipelineLayout.h"
#include "RayTracing/SbtWarpper.h"


class Device;

class Subpass;

class RenderPass;

class FrameBuffer;

// class SbtWarpper;


class Pipeline {
public:
    VkPipeline getHandle() const { return pipeline; }

    Pipeline(Device &device, const PipelineState &pipelineState);

    SbtWarpper &getSbtWarpper() { return *mSbtWarpper; }
protected:
    VkPipeline pipeline;
    Device &device;
    
    //For ray tracing pipeline 
    std::unique_ptr<SbtWarpper> mSbtWarpper{nullptr};
};

