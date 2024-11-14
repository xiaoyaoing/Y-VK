#pragma once
#include "RenderPassBase.h"
#include "Core/Texture.h"

//Implement of https://gpuopen.com/learn/mesh_shaders/mesh_shaders-procedural_grass_rendering/#appendix
//Aim to learn mesh shader and procedural grass rendering
class GrassPass : public PassBase{
public:
    void render(RenderGraph& rg) override;
    void updateGui() override;
    GrassPass();
protected:
    struct PushConstant;
    PushConstant * mPushConstant{nullptr};
    std::unique_ptr<Texture>  perlinNoise{nullptr};
};
