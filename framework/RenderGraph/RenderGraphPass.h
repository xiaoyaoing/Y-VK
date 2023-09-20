#pragma once
#include <Vulkan.h>

class CommandBuffer;
class RenderPass;

struct RenderPassContext
{
    RenderPass& renderPass;
    CommandBuffer& commandBuffer;
    VkPipeline pipeline;
    // PipelineS
};

class RenderGraphPassBase
{
public:
    virtual void execute(RenderPassContext& context);
};

template <typename Data, typename Execute>
class RenderGraphPass : public RenderGraphPassBase
{
    Data data;
    Execute mExecute;

public:
    void execute(RenderPassContext& context) override
    {
        mExecute(data, context);
    }

    Data& getData() { return data; }
};
