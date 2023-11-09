#pragma once

#include <Vulkan.h>

class RenderTarget;
class CommandBuffer;

class RenderPass;

struct RenderPassContext
{
    //RenderPass& renderPass;
    //CommandBuffer& commandBuffer;
    RenderTarget& renderTarget;
    //VkPipeline pipeline;

    // PipelineS
};

class RenderGraphPassBase
{
public:
    virtual void execute(RenderPassContext& context) = 0;
    virtual ~RenderGraphPassBase() = default;
};

template <typename Data, typename Execute>
class RenderGraphPass : public RenderGraphPassBase
{
    Data data;
    Execute mExecute;

public:
    explicit RenderGraphPass(Execute&& execute) noexcept
        : mExecute(std::move(execute))
    {
    }

    void execute(RenderPassContext& context) override
    {
        mExecute(data, context);
    }

    Data& getData() { return data; }
};
