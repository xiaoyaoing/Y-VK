#include "RuntimeSceneManager.h"

#include "Core/RenderContext.h"
#include "SceneLoader/SceneLoaderInterface.h"

#include <Scene/Scene.h>

uint32_t getIndexStride(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        default:
            throw std::runtime_error("Invalid index type");
    }
}

void RuntimeSceneManager::addPrimitives(Scene& scene, std::vector<std::unique_ptr<Primitive>>&& primitives) {
    auto&    device                    = g_context->getDevice();
    uint32_t vertexBufferRequiredCount = scene.getVertexBuffer(POSITION_ATTRIBUTE_NAME).getSize() / scene.vertexAttributes.at(POSITION_ATTRIBUTE_NAME).stride;
    uint32_t indexBufferRequiredSize   = scene.getIndexBuffer().getSize();
    for (auto& prim : primitives) {
        prim->firstIndex += indexBufferRequiredSize / getIndexStride(scene.getIndexType());
        prim->firstVertex += vertexBufferRequiredCount;

        vertexBufferRequiredCount += prim->getVertexBuffer(POSITION_ATTRIBUTE_NAME).getSize() / scene.vertexAttributes.at(POSITION_ATTRIBUTE_NAME).stride;
        indexBufferRequiredSize += prim->getIndexBuffer().getSize();
        // scene.primitives.push_back(std::move(prim));
    }

    std::unordered_map<std::string, std::unique_ptr<Buffer>> newVertexBuffers;
    std::unique_ptr<Buffer>                                  newIndexBuffer;

    CommandBuffer commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    for (auto& buffer : scene.sceneVertexBuffer) {
        //newVertexBuffers.emplace(buffer.first, std::make_unique<Buffer>(buffer.second->getDevice(), vertexBufferRequiredSize, buffer.second->getUsageFlags(), buffer.second->getMemoryUsage()));
        newVertexBuffers.emplace(buffer.first, Buffer::FromBuffer(device, commandBuffer, *buffer.second, buffer.second->getUsageFlags(), 0, vertexBufferRequiredCount * scene.vertexAttributes.at(buffer.first).stride));
    }
    newIndexBuffer = Buffer::FromBuffer(device, commandBuffer, scene.getIndexBuffer(), scene.getIndexBuffer().getUsageFlags(), 0, indexBufferRequiredSize);

    auto vertexCount = scene.getVertexBuffer(POSITION_ATTRIBUTE_NAME).getSize() / scene.vertexAttributes.at(POSITION_ATTRIBUTE_NAME).stride;
    auto indexCount  = scene.getIndexBuffer().getSize() / getIndexStride(scene.getIndexType());
    for (auto& prim : primitives) {
        prim->firstIndex  = indexCount;
        prim->firstVertex = vertexCount;
        {
            for (auto& attr : scene.vertexAttributes) {
                VkBufferCopy2 bufferCopy = {.srcOffset = 0, .dstOffset = vertexCount * attr.second.stride, .size = prim->getVertexBuffer(attr.first).getSize()};
                bufferCopy.sType         = VK_STRUCTURE_TYPE_BUFFER_COPY_2_KHR;
                VkCopyBufferInfo2 copyInfo{VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2_KHR};
                copyInfo.srcBuffer   = prim->getVertexBuffer(attr.first).getHandle();
                copyInfo.dstBuffer   = newVertexBuffers[attr.first]->getHandle();
                copyInfo.regionCount = 1;
                copyInfo.pRegions    = &bufferCopy;
                vkCmdCopyBuffer2(commandBuffer.getHandle(), &copyInfo);
            }
            vertexCount += prim->getVertexBuffer(POSITION_ATTRIBUTE_NAME).getSize() / scene.vertexAttributes.at(POSITION_ATTRIBUTE_NAME).stride;
        }

        {
            VkBufferCopy2     bufferCopy = {.srcOffset = 0, .dstOffset = indexCount * getIndexStride(scene.getIndexType()), .size = prim->getIndexBuffer().getSize()};
            VkCopyBufferInfo2 copyInfo{};
            copyInfo.srcBuffer   = prim->getIndexBuffer().getHandle();
            copyInfo.dstBuffer   = newIndexBuffer->getHandle();
            copyInfo.regionCount = 1;
            copyInfo.pRegions    = &bufferCopy;
            vkCmdCopyBuffer2(commandBuffer.getHandle(), &copyInfo);
            indexCount += prim->getIndexBuffer().getSize() / getIndexStride(scene.getIndexType());
        }
    }
    g_context->submit(commandBuffer);
    scene.addPrimitives(std::move(primitives));
    scene.sceneIndexBuffer  = std::move(newIndexBuffer);
    scene.sceneVertexBuffer = std::move(newVertexBuffers);
    scene.updateSceneUniformBuffer();
}
void RuntimeSceneManager::addPrimitive(Scene& scene, std::unique_ptr<Primitive>&& primitive) {
    auto primitives = std::vector<std::unique_ptr<Primitive>>();
    primitives.push_back(std::move(primitive));
    addPrimitives(scene, std::move(primitives));
}
void RuntimeSceneManager::addGltfMaterialsToScene(Scene& scene, std::vector<GltfMaterial>&& materials) {
    scene.materials.resize(scene.materials.size() + materials.size());
    std::copy(materials.begin(), materials.end(), scene.materials.end() - materials.size());
    // scene.material
}
void RuntimeSceneManager::addSponzaRestirLight(Scene& scene) {
    // Magic numbers used to offset lights in the Sponza scene
    auto light_pos   = glm::vec3(0.0f, 128.0f, -225.0f);
    auto light_color = glm::vec3(1.0, 1.0, 1.0);

    uint originalSize = scene.getPrimitives().size();

    {
        std::vector<std::string> types(48, "sphere");
        auto                     primitives = SceneLoaderInterface::loadSpecifyTypePrimitives(g_context->getDevice(), types);
        addPrimitives(scene, std::move(primitives));
    }
    //  scene.primitives.reserve(scene.primitives.size() + 48);
    // auto & sphere = scene.primitives[scene.primitives.size()-1];

    // for(int i = 0;i<71;i++){
    //     auto prim = std::make_unique<Primitive>(sphere->firstIndex, sphere->firstVertex, sphere->vertexCount, sphere->indexCount, sphere->materialIndex);
    //     scene.addPrimitive(std::move(prim));
    // }

    uint32_t idx = 0;
    for (int i = -4; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            glm::vec3 pos = light_pos;
            pos.x += i * 400;
            pos.z += j * (225 + 140);
            pos.y = 8;

            for (int k = 0; k < 3; ++k) {
                pos.y = pos.y + (k * 100);

                light_color.x = static_cast<float>(rand()) / (RAND_MAX);
                light_color.y = static_cast<float>(rand()) / (RAND_MAX);
                light_color.z = static_cast<float>(rand()) / (RAND_MAX);

                //  LightProperties props;
                // props.color      = light_color * 100.f;
                // props.intensity  = 1.f;
                // props.prim_index = idx + originalSize;
                // scene.addLight(SgLight{.type = LIGHT_TYPE::Area, .lightProperties = props});

                GltfMaterial material       = InitGltfMaterial();
                material.pbrBaseColorFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
                material.emissiveFactor     = light_color * 10.f;
                scene.materials.push_back(material);

                scene.primitives[originalSize + idx]->transform.setPosition(pos);
                scene.primitives[originalSize + idx]->transform.setLocalScale(glm::vec3(10.f));
                scene.primitives[originalSize + idx]->materialIndex = scene.materials.size() - 1;

                idx++;
            }
        }
    }

    scene.updateSceneUniformBuffer();
}
void RuntimeSceneManager::addSponzaRestirPointLight(Scene& scene) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            glm::vec3 pos = glm::vec3(-100.0f, 128.0f, -225.0f);
            pos.x += i * 400;
            pos.z += j * 140;
            pos.y = 8;

            LightProperties props;
            props.color      = glm::vec3(1.0f, 1.0f, 1.0f) * 100.f;
            props.intensity  = 1.f;
            props.position   = pos / 10.f;
            props.prim_index = 0;
            scene.addLight(SgLight{.type = LIGHT_TYPE::Point, .lightProperties = props});
        }
    }
}
void RuntimeSceneManager::addPlane(Scene& scene) {
    auto quad = SceneLoaderInterface::loadSpecifyTypePrimitive(g_context->getDevice(), "quad");
    quad->transform.setLocalScale(glm::vec3(100.f));
    GltfMaterial material       = InitGltfMaterial();
    material.pbrBaseColorFactor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    material.pbrRoughnessFactor = 1.0;
    material.pbrMetallicFactor = 0.0;
    
    RuntimeSceneManager::addPrimitive(scene, std::move(quad));
    scene.materials.push_back(material);
    scene.primitives[scene.primitives.size() - 1]->materialIndex = scene.materials.size() - 1;
    scene.updateSceneUniformBuffer();
}
