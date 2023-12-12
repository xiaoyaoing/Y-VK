//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"

class Example : public Application
{
public:
    void prepare() override;

    Example();

protected:
    bool useSubpass{true};


    struct
    {
        std::unique_ptr<PipelineLayout> gBuffer, lighting;
    } pipelineLayouts;


    void onUpdateGUI() override;
    void drawFrame(RenderGraph &renderGraph) override;
};
