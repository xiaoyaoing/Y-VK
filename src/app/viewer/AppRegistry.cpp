#include "AppRegistry.h"

#include "App/Application.h"
#include "Common/FIleUtils.h"
#include "Common/ResourceCache.h"
#include "render_modules/pbr/PbrLab.h"
#include "render_modules/raytracing/RayTracer.h"
#include "render_modules/vxgi/Vxgi.h"

namespace {

std::unique_ptr<Application> createRayTracerApp() {
    Json config = JsonUtil::fromFile(FileUtils::getResourcePath("render.json"));
    return std::make_unique<RayTracer>(config);
}

const std::vector<AppDescriptor> kAppRegistry = {
    {"raytracer", "Hybrid ray tracing renderer", createRayTracerApp},
    {"vxgi", "Voxel cone tracing renderer", [] { return std::make_unique<VXGI>(); }},
    {"pbr", "PBR and IBL renderer", [] { return std::make_unique<PBRLab>(); }},
};

} // namespace

const std::vector<AppDescriptor>& getAppRegistry() {
    return kAppRegistry;
}

std::unique_ptr<Application> createAppByName(std::string_view name) {
    for (const auto& app : kAppRegistry) {
        if (app.name == name) {
            return app.create();
        }
    }
    return nullptr;
}
