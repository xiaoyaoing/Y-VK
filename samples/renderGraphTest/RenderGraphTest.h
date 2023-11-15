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


    std::unique_ptr<gltfLoading::Model> sponza;

    struct
    {
        std::unique_ptr<PipelineLayout> gBuffer, lighting;
    } pipelineLayouts;


    void onUpdateGUI() override;


    void drawFrame() override;
};
