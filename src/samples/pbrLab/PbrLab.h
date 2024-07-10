//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"
#include "RenderIng/IBL.h"
#include "RenderPasses/RenderPassBase.h"

class Example : public Application {
public:
    void prepare() override;

    Example();

protected:
    std::vector<std::unique_ptr<PassBase>> mRenderPasses;
    void                                   onUpdateGUI() override;
    void                                   drawFrame(RenderGraph& renderGraph) override;
    std::unique_ptr<IBL>                   ibl;
    std::unique_ptr<Primitive>             cube;
    std::unique_ptr<Texture>               environmentCube;
    float                                  exposure = 4.5f;
    float                                  gamma    = 2.2f;
    glm::vec3 dir = vec3(1);

};