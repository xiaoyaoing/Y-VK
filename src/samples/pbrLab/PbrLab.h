//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"
#include "RenderPasses/RenderPassBase.h"

class Example : public Application {
public:
    void prepare() override;

    Example();

protected:
    std::vector<std::unique_ptr<PassBase>> mRenderPasses;
    void                                   onUpdateGUI() override;
    void                                   drawFrame(RenderGraph& renderGraph) override;
};