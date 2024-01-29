#include "View.h"

#include "RenderContext.h"
#include "Scene/Scene.h"
#include "Scene/Compoments/SgLight.h"

constexpr size_t CONFIG_MAX_LIGHT_COUNT = 256;

View::View(Device& device) {
    mLightBuffer   = std::make_unique<Buffer>(device, sizeof(LightUib) * CONFIG_MAX_LIGHT_COUNT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    mPerViewBuffer = std::make_unique<Buffer>(device, sizeof(PerViewUnifom), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
}
void View::setScene(const Scene* scene) {
    mScene = scene;
    std::vector<LightUib> lights;
    lights.reserve(CONFIG_MAX_LIGHT_COUNT);
    //uint32_t curLightCount = 0;
    for (const auto& light : mScene->getLights()) {
        LightUib LightUib{};
        switch (light.type) {
            case LIGHT_TYPE::Point:
                LightUib = {
                    .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                    .position  = glm::vec4(light.lightProperties.position, 1),
                    .direction = glm::vec4(0.0f),
                    .info      = glm::vec4(0.0f)};
                lights.push_back(LightUib);
            default:
                break;//todo
        }
    }
    mLightBuffer->uploadData(lights.data(), mLightBuffer->getSize(), 0);
    for (const auto& primitive : mScene->getPrimitives()) {
        mVisiblePrimitives.emplace_back(primitive.get());
    }
}
void View::setCamera(const Camera* camera) {
    mCamera = camera;
}
void View::bindViewBuffer() {
    PerViewUnifom perViewUnifom{};
    perViewUnifom.view_proj      = mCamera->matrices.perspective * mCamera->matrices.view;
    perViewUnifom.inv_view_proj  = glm::inverse(perViewUnifom.view_proj);
    perViewUnifom.resolution     = glm::ivec2(g_context->getSwapChainExtent().width, g_context->getSwapChainExtent().height);
    perViewUnifom.inv_resolution = glm::vec2(1.0f / perViewUnifom.resolution.x, 1.0f / perViewUnifom.resolution.y);
    perViewUnifom.light_count    = mScene->getLights().size();

    //perViewUnif
    mPerViewBuffer->uploadData(&perViewUnifom, sizeof(PerViewUnifom), 0);

    RenderContext* context = g_context;
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::LIGHTS), *mLightBuffer, 0, mLightBuffer->getSize(), 0);
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::PER_VIEW), *mPerViewBuffer, 0, mPerViewBuffer->getSize(), 0);
}
const Camera* View::getMCamera() const {
    return mCamera;
}
void View::setMCamera(const Camera* const mCamera) {
    this->mCamera = mCamera;
}
std::vector<const Primitive*> View::getMVisiblePrimitives() const {
    return mVisiblePrimitives;
}
void View::setMVisiblePrimitives(const std::vector<const Primitive*>& mVisiblePrimitives) {
    this->mVisiblePrimitives = mVisiblePrimitives;
}