//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"
#include "Rendering/IBL.h"
#include "RenderPasses/RenderPassBase.h"
/**
 * @class PBRLab
 * @brief A sample application demonstrating PBR (Physically Based Rendering) techniques.
 *
 * This class extends the Application base class to create a PBR rendering example.
 * It includes features such as:
 * - Image-Based Lighting (IBL)
 * - Multiple render passes
 * - Environment mapping
 * - Exposure and gamma controls
 *
 * The class sets up the necessary components for PBR rendering, including
 * render passes, IBL setup, and primitive objects (like a cube).
 * It also provides methods for updating the GUI and drawing frames using a render graph.
 */

class PBRLab : public Application {
public:
    void prepare() override;

    PBRLab();

protected:
    std::vector<std::unique_ptr<PassBase>> mRenderPasses;
    std::vector<std::unique_ptr<PassBase>> mforwardRenderPasses;
    void                                   onUpdateGUI() override;
    void                                   drawFrame(RenderGraph& renderGraph) override;
    std::unique_ptr<IBL>                   ibl;
    std::unique_ptr<Primitive>             cube;
    std::unique_ptr<Texture>               environmentCube{nullptr};
    std::unique_ptr<Texture>               environmentCubeAsync {nullptr};
    float                                  exposure = 4.5f;
    float                                  gamma    = 2.2f;
};