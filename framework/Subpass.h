//
// Created by pc on 2023/8/4.
//
#include <Vulkan.h>
#include "CommandBuffer.h"

#pragma once

class RenderTarget;

class Mesh;

class Subpass {
public:
    void updateRenderTargetAttachments(RenderTarget &renderTarget);

    virtual void draw(CommandBuffer &commandBuffer) = 0;
};


class geomSubpass : public Subpass {
public:
    virtual void draw(CommandBuffer &commandBuffer) override;

private:
    std::vector<Mesh *> meshes;
};


