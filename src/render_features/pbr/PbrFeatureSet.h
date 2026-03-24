#pragma once

#include "RenderPasses/RenderPassBase.h"
#include "Rendering/IBL.h"

class Device;
class Gui;
class Primitive;
class RenderContext;
class RenderGraph;
class Texture;
class View;

class PbrFeatureSet {
public:
    void initialize(Device& device);
    void render(RenderGraph& rg, RenderContext& renderContext, Device& device, View& view, float exposure, float gamma);
    void updateGui(Gui& gui);

private:
    std::vector<std::unique_ptr<PassBase>> mRenderPasses;
    std::vector<std::unique_ptr<PassBase>> mForwardRenderPasses;
    std::unique_ptr<IBL>                   ibl;
    std::unique_ptr<Primitive>             cube;
    std::unique_ptr<Texture>               environmentCube;
    std::unique_ptr<Texture>               environmentCubeAsync;
};
