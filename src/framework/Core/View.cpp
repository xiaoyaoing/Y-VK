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
        LightUib lightUib;
        switch (light.type) {
            case LIGHT_TYPE::Point:
                lightUib = {
                    .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                    .position  = glm::vec4(light.lightProperties.position, 1),
                    .direction = glm::vec4(0.0f),
                    .info      = glm::vec4(2)};
            case LIGHT_TYPE::Directional:
                lightUib = {
                    .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                    .direction = glm::vec4(light.lightProperties.direction, 1),
                    .info      = glm::vec4(1)};
            default:
                break;//todo
        }
        lights.emplace_back(lightUib);
    }
    mLightBuffer->uploadData(lights.data(), mLightBuffer->getSize(), 0);
    for (const auto& primitive : mScene->getPrimitives()) {
        mVisiblePrimitives.emplace_back(primitive.get());
    }
    for (const auto& texture : mScene->getTextures()) {
        mTextures.emplace_back(texture.get());
    }
    mMaterials = scene->getGltfMaterials();
}
void View::setCamera(const Camera* camera) {
    mCamera = camera;
}

View& View::bindViewBuffer() {
    PerViewUnifom perViewUnifom{};
    perViewUnifom.view_proj     = mCamera->viewProj();
    perViewUnifom.proj          = mCamera->proj();
    perViewUnifom.view          = mCamera->view();
    perViewUnifom.inv_view_proj = glm::inverse(perViewUnifom.view_proj);

    vec3 position = vec3(mCamera->getPosition() + 1.f);
    //auto p = glm::vec4(position,1) * perViewUnifom.view_proj * perViewUnifom.inv_view_proj;

    //  assert(glm::vec4(position,1) * perViewUnifom.view_proj * perViewUnifom.inv_view_proj == glm::vec4(position,1));

    perViewUnifom.resolution     = glm::ivec2(g_context->getSwapChainExtent().width, g_context->getSwapChainExtent().height);
    perViewUnifom.inv_resolution = glm::vec2(1.0f / perViewUnifom.resolution.x, 1.0f / perViewUnifom.resolution.y);
    perViewUnifom.light_count    = mScene->getLights().size();
    perViewUnifom.camera_pos     = mCamera->getPosition();

    if (perViewUnifom != mPerViewUniform) {
        mPerViewBuffer->uploadData(&perViewUnifom, sizeof(PerViewUnifom), 0);
        mPerViewUniform = perViewUnifom;
    }
    //perViewUnif

    RenderContext* context = g_context;
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::LIGHTS), *mLightBuffer, 0, mLightBuffer->getSize(), 0);
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::PER_VIEW), *mPerViewBuffer, 0, mPerViewBuffer->getSize(), 0);

    return *this;
}
View& View::bindViewShading() {
    auto             materials  = GetMMaterials();
    BufferAllocation allocation = g_context->allocateBuffer(sizeof(GltfMaterial) * materials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    allocation.buffer->uploadData(materials.data(), allocation.size, allocation.offset);
    g_context->bindBuffer(3, *allocation.buffer, allocation.offset, 0);

    const auto& textures = GetMTextures();
    for (uint32_t i = 0; i < textures.size(); i++) {
        g_context->bindImageSampler(6, textures[i]->getImage().getVkImageView(), textures[i]->getSampler(), 0, i);
    }

    return *this;
}
View& View::bindViewGeom(CommandBuffer& commandBuffer) {
    g_context->bindScene(commandBuffer, *mScene);
    return *this;
}
void View::drawPrimitives(CommandBuffer& commandBuffer) {
    uint32_t instance_count = 0;
    for (const auto& primitive : mVisiblePrimitives) {
        g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, primitive->firstVertex, instance_count++);
    }
}
void View::drawPrimitives(CommandBuffer& commandBuffer, const PrimitiveSelectFunc& selectFunc) {
    uint32_t instance_count = 0;
    for (const auto& primitive : mVisiblePrimitives) {
        if (selectFunc(*primitive)) {
            g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, primitive->firstVertex, instance_count);
        }
        instance_count++;
    }
}

const Camera* View::getCamera() const {
    return mCamera;
}

std::vector<const Primitive*> View::getMVisiblePrimitives() const {
    return mVisiblePrimitives;
}
void View::setMVisiblePrimitives(const std::vector<const Primitive*>& mVisiblePrimitives) {
    this->mVisiblePrimitives = mVisiblePrimitives;
}
std::vector<const Texture*> View::GetMTextures() const {
    return mTextures;
}
std::vector<GltfMaterial> View::GetMMaterials() const {
    return mMaterials;
}