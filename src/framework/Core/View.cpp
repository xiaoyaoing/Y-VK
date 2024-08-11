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

    lightDirty = true;
    for (const auto& primitive : mScene->getPrimitives()) {
        mVisiblePrimitives.emplace_back(primitive.get());
    }
    for (const auto& texture : mScene->getTextures()) {
        mImageViews.emplace_back(&texture->getImage().getVkImageView());
        mSamplers.emplace_back(&texture->getSampler());
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
    perViewUnifom.inv_proj      = glm::inverse(perViewUnifom.proj);
    perViewUnifom.inv_view      = glm::inverse(perViewUnifom.view);

    perViewUnifom.resolution     = glm::ivec2(g_context->getViewPortExtent().width, g_context->getViewPortExtent().height);
    perViewUnifom.inv_resolution = glm::vec2(1.0f / perViewUnifom.resolution.x, 1.0f / perViewUnifom.resolution.y);
    perViewUnifom.light_count    = mScene->getLights().size();
    perViewUnifom.camera_pos     = mCamera->getPosition();

    if (perViewUnifom != mPerViewUniform) {
        mPerViewBuffer->uploadData(&perViewUnifom, sizeof(PerViewUnifom), 0);
        mPerViewUniform = perViewUnifom;
    }
    //perViewUnif

    RenderContext* context = g_context;

    if(lightDirty) updateLight();
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::LIGHTS), *mLightBuffer, 0, mLightBuffer->getSize(), 0);
    context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::PER_VIEW), *mPerViewBuffer, 0, mPerViewBuffer->getSize(), 0);

    return *this;
}

View& View::bindViewShading() {
    auto             materials  = GetMMaterials();
    BufferAllocation allocation = g_context->allocateBuffer(sizeof(GltfMaterial) * materials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    allocation.buffer->uploadData(materials.data(), allocation.size, allocation.offset);
    g_context->bindBuffer(3, *allocation.buffer, allocation.offset, 0);

    for (uint32_t i = 0; i < mImageViews.size(); i++) {
        g_context->bindImageSampler(i, *mImageViews[i], *mSamplers[i]);
    }

    //bind shadow maps
    return *this;
}
int View::bindTexture(const ImageView& imageView, const Sampler& sampler) {
    mImageViews.emplace_back(&imageView);
    mSamplers.emplace_back(&sampler);
    return mImageViews.size() - 1;
}

View& View::bindViewGeom(CommandBuffer& commandBuffer) {
    if (mScene->getBufferRate() == BufferRate::PER_SCENE)
        g_context->bindScene(commandBuffer, *mScene);
    return *this;
}
void View::drawPrimitives(CommandBuffer& commandBuffer) {
    if (mScene->getBufferRate() == BufferRate::PER_PRIMITIVE) {
        drawPrimitivesUseSeparateBuffers(commandBuffer);
        return;
    }
    uint32_t instance_count = 0;
    for (const auto& primitive : mVisiblePrimitives) {
        g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, primitive->firstVertex, instance_count++);
    }
}
void View::drawPrimitives(CommandBuffer& commandBuffer, const PrimitiveSelectFunc& selectFunc) {
    int instance_count = 0;
    for (const auto& primitive : mVisiblePrimitives) {
        if (selectFunc(*primitive)) {
            g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, primitive->firstVertex, instance_count);
        }
        instance_count++;
    }
}
void View::drawPrimitivesUseSeparateBuffers(CommandBuffer& commandBuffer) {
    uint32_t instance_count = 0;
    //Here first binding hard code
    //Fix me
    commandBuffer.bindVertexBuffer(3, mScene->getPrimitiveIdBuffer(), {0});
    g_context->bindBuffer(static_cast<uint32_t>(UniformBindingPoints::PRIM_INFO), mScene->getUniformBuffer(), 0, mScene->getUniformBuffer().getSize());
    for (const auto& primitive : mVisiblePrimitives) {
        g_context->bindPrimitiveGeom(commandBuffer, *primitive);
        g_context->flushAndDrawIndexed(commandBuffer, primitive->indexCount, 1, 0, 0, instance_count++);
        //  return;
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
std::vector<GltfMaterial> View::GetMMaterials() const {
    return mMaterials;
}
void View::updateLight() {
    std::vector<LightUib> lights;
    lights.reserve(mScene->getLights().size());
    //uint32_t curLightCount = 0;
    for (const auto& light : mScene->getLights()) {
        LightUib lightUib;
        lightUib.info.g = light.lightProperties.shadow_index;
        switch (light.type) {
            case LIGHT_TYPE::Point:
                lightUib = {
                .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                .position  = glm::vec4(light.lightProperties.position, 1),
                .direction = glm::vec4(0.0f),
                .info      = glm::vec4(2)};
            break;
            case LIGHT_TYPE::Directional:
                lightUib = {
                .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                .direction = glm::vec4(light.lightProperties.direction, 1),
                .info      = glm::vec4(1)};
            break;
            case LIGHT_TYPE::Spot:
                lightUib = {
                .color     = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                .position  = glm::vec4(light.lightProperties.position, 1),
                .direction = glm::vec4(light.lightProperties.direction, 2),
                .info      = glm::vec4(light.lightProperties.inner_cone_angle, light.lightProperties.outer_cone_angle, 0, 0)};
            break;
            default:
                break;//todo
        }
        lights.emplace_back(lightUib);
    }
    mLightBuffer->uploadData(lights.data(), mLightBuffer->getSize(), 0);
    lightDirty = false;
}