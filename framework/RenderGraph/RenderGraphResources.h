#pragma once
#include "RenderGraphTexture.h"


class RenderGraph;
class PassNode;

class RenderGraphResources
{
public:
    struct RenderPassInfo
    {
        HwTexture texture;
    };

    RenderPassInfo getRenderPassInfo() const;

private:
    RenderGraph& renderGraph;
    PassNode& passNode;
};
