#include "SceneLoaderInterface.h"

#include "ObjLoader.hpp"
#include "gltfloader.h"
#include "jsonloader.h"
#include "Core/RenderContext.h"

static std::unordered_map<std::string, std::function<std::unique_ptr<Scene>(Device& device, const std::string& path, const SceneLoadingConfig& config)>> sceneLoaders = {
    {"json", Jsonloader::LoadSceneFromJsonFile},
    {"gltf", GltfLoading::LoadSceneFromGLTFFile}};
std::unique_ptr<Scene> SceneLoaderInterface::LoadSceneFromFile(Device& device, const std::string& path, const SceneLoadingConfig& config) {
    std::string extension = path.substr(path.find_last_of(".") + 1);
    return sceneLoaders[extension](device, path, config);
}

std::unique_ptr<Primitive> SceneLoaderInterface::loadSpecifyTypePrimitive(Device& device, const std::string& type) {
    auto primitives = loadSpecifyTypePrimitives(device, {type});
    return std::move(primitives[0]);
}
std::vector<std::unique_ptr<Primitive>> SceneLoaderInterface::loadSpecifyTypePrimitives(Device& device, const std::vector<std::string>& types) {
    std::vector<std::unique_ptr<Primitive>> primitives;
    auto                                    cmdBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    for (auto& type : types) {
        auto                                                     primitiveData = PrimitiveLoader::loadPrimitiveFromType(type);
        uint32_t                                                 vertexCount   = primitiveData->buffers.at(POSITION_ATTRIBUTE_NAME).size() / sizeof(glm::vec3);
        uint32_t                                                 indexCount    = primitiveData->indexs.size() / sizeof(uint32_t);
        auto                                                     primitive     = std::make_unique<Primitive>(0, 0, vertexCount, indexCount);
        std::unordered_map<std::string, std::unique_ptr<Buffer>> stagingBuffers;
        for (auto attr : primitiveData->vertexAttributes) {
            primitive->setVertxAttribute(attr.first, attr.second);
            stagingBuffers[attr.first] = std::make_unique<Buffer>(device, primitiveData->buffers[attr.first].size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, primitiveData->buffers[attr.first].data());
        }
        auto stagingIndexBuffer = std::make_unique<Buffer>(device, primitiveData->indexs.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, primitiveData->indexs.data());
        for (auto attr : primitiveData->vertexAttributes) {
            auto buffer = Buffer::FromBuffer(device, cmdBuffer, *stagingBuffers[attr.first], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            primitive->setVertexBuffer(attr.first, buffer);
        }
        auto indexBuffer = Buffer::FromBuffer(device, cmdBuffer, *stagingIndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
        primitive->setIndexBuffer(indexBuffer);
        primitive->setIndexType(VK_INDEX_TYPE_UINT32);
        primitives.push_back(std::move(primitive));
    }
    g_context->submit(cmdBuffer);
    return primitives;
}
