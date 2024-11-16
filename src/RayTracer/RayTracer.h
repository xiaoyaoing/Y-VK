//
// Created by pc on 2023/12/1.
//

#ifndef VULKANDEMO_RAYTRACER_H
#define VULKANDEMO_RAYTRACER_H

#include "App/Application.h"

#include <Core/RayTracing/Accel.h>

#include "Integrators/Integrator.h"
#include "PostProcess/PostProcess.h"
#include "RenderPasses/RenderPassBase.h"

\

#define RESTIR_INTEGRATOR_NAME "restir"
#define PATH_INTEGRATOR_NAME   "path"

class RayTracer : public Application {
public:
    RayTracer(const RTConfing& settings);
    void prepare() override;
    void onUpdateGUI() override;
    void drawFrame(RenderGraph& renderGraph) override;
    void onSceneLoaded() override;

protected:
    void perFrameUpdate() override;

    std::string getHdrImageToSave() override;

public:
    // virtual void update() override;

    // std::unique_ptr<Integrator> path,restirDI{};

    std::unordered_map<std::string, std::unique_ptr<Integrator>> integrators;
    std::string                                                  currentIntegrator = RESTIR_INTEGRATOR_NAME;
    std::vector<std::string>
        integratorNames;

    struct
    {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
    } cameraUbo;

    SceneUbo                      sceneUbo;
    SceneUbo                      lastFrameSceneUbo;
    std::shared_ptr<RTSceneEntry> rtSceneEntry;
    std::shared_ptr<PCPath> pcPath;
    std::unique_ptr<Texture> asyncEnvironmenMap{nullptr};
    // std::unique_ptr<PostProcess>  postProcess;
};

#endif//VULKANDEMO_RAYTRACER_H
