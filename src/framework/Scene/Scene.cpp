#include "Scene.h"

#include "Compoments/Camera.h"

Material Material::getDefaultMaterial() {
    Material material{};
    return material;
}
void Scene::IteratePrimitives(PrimitiveCallBack primitiveCallBack) const {
    for (const auto& prim : primitives) {
        primitiveCallBack(*prim);
    }
}

Scene::Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<Material>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras)
    : materials(std::move(materials)), lights(std::move(lights)), primitives(std::move(primitives)), textures(std::move(textures)), cameras(std::move(cameras)) {
    for (auto& prim : this->primitives) {
        assert(prim->valid());
    }
}

Scene::Scene(std::vector<std::unique_ptr<Primitive>>&& primitives, std::vector<std::unique_ptr<Texture>>&& textures, std::vector<GltfMaterial>&& materials, std::vector<SgLight>&& lights, std::vector<std::shared_ptr<Camera>>&& cameras)
    : gltfMaterials(std::move(materials)), lights(std::move(lights)), primitives(std::move(primitives)), textures(std::move(textures)), cameras(std::move(cameras)) {
    for (auto& prim : this->primitives) {
        assert(prim->valid());
    }
}

void Scene::addLight(const SgLight& light) {
    lights.emplace_back(light);
}
void Scene::addDirectionalLight(vec3 direction, vec3 color, float intensity) {
    SgLight light{};
    light.lightProperties.color     = color;
    light.lightProperties.intensity = intensity;
    light.lightProperties.direction = direction;
    light.type                      = LIGHT_TYPE::Directional;
    lights.emplace_back(light);
}

const std::vector<SgLight>& Scene::getLights() const {
    return lights;
}

const std::vector<std::unique_ptr<Primitive>>& Scene::getPrimitives() const {
    return primitives;
}

const std::vector<std::unique_ptr<Texture>>& Scene::getTextures() const {
    return textures;
}


const std::vector<GltfMaterial>& Scene::getGltfMaterials() const {
    return gltfMaterials;
}

std::vector<std::shared_ptr<Camera>>& Scene::getCameras() {
    return cameras;
}

bool Primitive::Validate() const {
    return  vertexBuffers.size() > 0 && indexBuffer != nullptr && uniformBuffer != nullptr && vertexCount > 0 && indexCount > 0;
}
bool Primitive::getVertexAttribute(const std::string& name, VertexAttribute * attribute) const {
    if (vertexAttributes.contains(name)) {
        if(attribute != nullptr)
            *attribute = vertexAttributes.at(name);
        return true;
    }
    return false;
}

void Primitive::setVertxAttribute(const std::string& name, const VertexAttribute& attribute) {
    vertexAttributes.emplace(name, attribute);
}

void Primitive::setVertexBuffer(const std::string& name, std::unique_ptr<Buffer>& buffer) {
    vertexBuffers.emplace(name, std::move(buffer));
}
void Primitive::setUniformBuffer(std::unique_ptr<Buffer>& buffer) {
    this->uniformBuffer = std::move(buffer);
}
void Primitive::setIndexBuffer(std::unique_ptr<Buffer>& buffer) {
    this->indexBuffer = std::move(buffer);
}
bool Primitive::valid() const {
    return uniformBuffer != nullptr && indexBuffer != nullptr && !vertexBuffers.empty();
}

Buffer& Primitive::getVertexBuffer(const std::string& name) const {
    if (vertexBuffers.contains(name))
        return *vertexBuffers.at(name);
    LOGE("Primitive has no such buffer {}", name);
}
VkIndexType Primitive::getIndexType() const {
    return indexType;
}

void Primitive::setIndexType(VkIndexType indexType) {
    this->indexType = indexType;
}
bool Primitive::hasVertexBuffer(const std::string& name) const {
    return vertexBuffers.contains(name);
}

const Buffer& Primitive::getIndexBuffer() const {
    return *indexBuffer;
}

const Buffer& Primitive::getUniformBuffer() const {
    return *uniformBuffer;
}

std::unique_ptr<Scene> loadDefaultTriangleScene(Device& device) {
    Material                                mat        = Material::getDefaultMaterial();
    std::vector<std::unique_ptr<Primitive>> primitives = {};
    primitives.push_back(std::make_unique<Primitive>(0, 0, 3, 0));

    const uint32_t bufferSize      = sizeof(float) * 3 * 3;
    const uint32_t indexBufferSize = sizeof(uint32_t) * 3;

    std::vector<glm::vec3> position = {{1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
    std::vector<glm::vec3> color    = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    std::vector<uint32_t>  index    = {0, 1, 2};
    PerPrimitiveUniform    uniform  = {.model = glm::mat4(1.0f)};

    auto positionBuffer = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto colorBuffer    = std::make_unique<Buffer>(device, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto indexBuffer    = std::make_unique<Buffer>(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto uniformBuffer  = std::make_unique<Buffer>(device, sizeof(uniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    positionBuffer->uploadData(position.data());
    colorBuffer->uploadData(color.data());
    indexBuffer->uploadData(index.data());
    uniformBuffer->uploadData(&uniform);

    auto& prim = primitives[0];
    prim->setIndexBuffer(indexBuffer);
    prim->setVertexBuffer(POSITION_ATTRIBUTE_NAME, positionBuffer);
    prim->setVertexBuffer("color", colorBuffer);
    prim->setUniformBuffer(uniformBuffer);
    prim->setVertxAttribute(POSITION_ATTRIBUTE_NAME, VertexAttribute{.format = VK_FORMAT_R32G32B32_SFLOAT, .stride = sizeof(float) * 3, .offset = 0});
    prim->setVertxAttribute("color", VertexAttribute{.format = VK_FORMAT_R32G32B32_SFLOAT, .stride = sizeof(float) * 3, .offset = 0});
    prim->setIndexType(VK_INDEX_TYPE_UINT32);
    auto scene = new Scene(std::move(primitives), {}, {mat}, {}, {});
    return std::unique_ptr<Scene>(scene);
}