#pragma once
#include "Scene/gltfloader.h"

class CommandBuffer;

class RenderContext;

class RenderPassDriver;

class RenderPassExecutor
{
public:
    RenderPassExecutor() = default;
    virtual void draw(CommandBuffer& commandBuffer, RenderContext& renderContext, gltfLoading::Model& model);
    virtual void updateUniformBuffer();
    virtual ~RenderPassExecutor() = default;
};


class GeomRenderPass : public RenderPassExecutor
{
};
