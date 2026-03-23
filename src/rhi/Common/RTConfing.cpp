#include "RTConfing.h"


using IntegratorKey = std::string;
static const IntegratorKey kPathIntegrator     = "path";
static const IntegratorKey kRestirDIIntegrator = "restir";
static const IntegratorKey kDDGIIntegrator     = "ddgi";

static const std::unordered_map<EIntegraotrType, IntegratorKey> kIntegratorTypeToString = {
    {ePathTracing, kPathIntegrator},
    {eDDGI, kDDGIIntegrator},
    {eRestirDI, kRestirDIIntegrator}
};

std::string to_string(EIntegraotrType type) {
    return kIntegratorTypeToString.at(type);
}

std::string RTConfing::getScenePath() const {
    return scenePath;
}

DDGIConfig RTConfing::getDDGIConfig() const {
    return ddgiConfig;
}
PathTracingConfig RTConfing::getPathTracingConfig() const {
    return pathTracingConfig;
}
EIntegraotrType RTConfing::getIntegratorType() const {
    return mIntegratorType;
}
void RTConfing::getSceneLoadingConfig(SceneLoadingConfig& config) const {
    {
        config.sceneScale = GetOptional(json, "scene_scale", config.sceneScale);
        config.sceneRotation = GetOptional(json, "scene_rotation",config.sceneRotation);
        config.sceneTranslation = GetOptional(json, "scene_translation", config.sceneTranslation);
    }
}
int RTConfing::getWindowWidth() const {
    return window_width;
}
int RTConfing::getWindowHeight() const {
    return window_height;
}
RTConfing::RTConfing(const Json& json) {
    this->json              = json;
    init();
}
RTConfing::RTConfing(const std::string& path) {
    json = JsonUtil::fromFile(path);
    init();
}
void RTConfing::init() {
    auto integratorsTypeStr = json["integrators"];
    for (auto& integratorJson : integratorsTypeStr) {
        auto integratorTypeStr = integratorJson["type"].get<std::string>();
        if (integratorTypeStr == kPathIntegrator) {
            pathTracingConfig.min_depth = GetOptional(integratorJson, "min_depth", pathTracingConfig.min_depth);
            pathTracingConfig.max_depth = GetOptional(integratorJson, "max_depth", pathTracingConfig.max_depth);
        } else if (integratorTypeStr == kDDGIIntegrator) {
            ddgiConfig.rays_per_probe = GetOptional(integratorJson, "ray_per_probe", ddgiConfig.rays_per_probe);
            ddgiConfig.probe_distance = GetOptional(integratorJson, "probe_distance", ddgiConfig.probe_distance);
            ddgiConfig.normal_bias    = GetOptional(integratorJson, "normal_bias", ddgiConfig.normal_bias);
            ddgiConfig.use_rt_gbuffer = GetOptional(integratorJson, "use_rt_gbuffer", ddgiConfig.use_rt_gbuffer);
        } else if (integratorTypeStr == kRestirDIIntegrator) {
            // integratorType = eRestirDI;
        }
    }

    auto integrator     = json["integrator"];
    auto integratorType = integrator["type"].get<std::string>();
    if (integratorType == kPathIntegrator) {
        mIntegratorType = ePathTracing;
    } else if (integratorType == kDDGIIntegrator) {
        mIntegratorType = eDDGI;
    } else if (integratorType == kRestirDIIntegrator) {
        mIntegratorType = eRestirDI;
    }

    {
        scenePath = json["scene_path"].get<std::string>();
    }
}