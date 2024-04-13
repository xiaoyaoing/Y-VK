#include "jsonloader.h"

#include "LoaderHelper.h"
#include "ObjLoader.hpp"
#include "Core/BufferPool.h"
#include "Core/RenderContext.h"
#include "Core/math.h"
#include "Raytracing/commons.h"
#include <Scene/Compoments/Camera.h>

#include <nlohmann/json.hpp>
#include <tiny_obj_loader.h>

using Json = nlohmann::json;

struct JsonLoader {
    JsonLoader(Device& device) : device(device) {}
    void LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config);
    void loadCamera();
    void PreprocessMaterials();
    void loadMaterials();
    void loadPrimitives();
    void Valiation();
    Json sceneJson;
    Device & device;
    
    std::vector<PerPrimitiveUniform> primitiveUniforms;
    
    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<std::unique_ptr<Texture>>   textures;
    std::vector<GltfMaterial>               materials;
    std::vector<RTMaterial>                 rtMaterials;
    std::vector<SgLight>                    lights;
    std::vector<std::shared_ptr<Camera>>    cameras;
    // std::vector<RTMaterial> rtMaterials;
    std::vector<Json> materialJsons;
    std::unordered_map<std::string,uint32_t>  materialIndexMap;
    std::unordered_map<std::string, VertexAttribute>         vertexAttributes;


    std::unordered_map<std::string, std::unique_ptr<Buffer>> sceneVertexBuffer;
    std::unique_ptr<Buffer>                                  sceneIndexBuffer{nullptr};
    std::unique_ptr<Buffer>                                  sceneUniformBuffer{nullptr};
    std::unique_ptr<Buffer>                                  scenePrimitiveIdBuffer{nullptr};

    std::filesystem::path rootPath;
};

namespace glm{
    void from_json(const Json& j, vec3& v) {
        if(!j.is_array()) {
            v = vec3(j.get<float>());
            return;
        }
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
    }
}


template <typename T>
T GetOptional(const Json& j, const std::string& key, const T& defaultValue) {
    if (j.contains(key)) {
        return j[key];
    }
    return defaultValue;
}

void JsonLoader::LoadSceneFromGLTFFile(Device& device, const std::string& path, const SceneLoadingConfig& config) {
    std::ifstream file(path);
    file >> sceneJson ;
    rootPath = std::filesystem::path(path).parent_path();
    PreprocessMaterials();
    loadCamera();
    loadPrimitives();
    loadMaterials();
    
}
void JsonLoader::loadCamera() {
    std::shared_ptr<Camera> camera     = std::make_shared<Camera>();
    Json                    cameraJson = sceneJson["camera"];
    camera->setTranslation(GetOptional(cameraJson, "translation", glm::vec3(12,-4,2)));
    camera->setRotation(GetOptional(cameraJson, "rotation", glm::vec3(0.0f)));
    camera->setPerspective(GetOptional(cameraJson, "fov", 45.0f), GetOptional(cameraJson, "aspect", 1.0f), GetOptional(cameraJson, "zNear", 0.1f), GetOptional(cameraJson, "zFar", 4000.0f));
    camera->flipY = true;
    cameras.push_back(camera);
}
void JsonLoader::PreprocessMaterials() {
    Json materialsJson = sceneJson["bsdfs"];
    for(auto & materialJson : materialsJson) {
        materialJsons.push_back(materialJson);
        if(materialJson.contains("name")) {
            materialIndexMap[materialJson["name"].get<std::string>()] = materialJsons.size() - 1;
        }
    }
    // for(auto & primitiveJson : sceneJson["primitives"]) {
    //     if(primitiveJson.contains("bsdf")) {
    //         auto &  materialJson = primitiveJson["bsdf"];
    //         if(ma)
    //     }
    // }
}

std::unordered_map<std::string,uint32_t> type2RTBSDFTYPE = {
    {"diffuse", RT_BSDF_TYPE_DIFFUSE},
    {"specular", RT_BSDF_TYPE_MIRROR}
};

void JsonLoader::loadMaterials() {
    std::unordered_set<std::string> texture_paths;
    for(auto & materialJson : materialJsons) {
        RTMaterial rtMaterial;
        if(materialJson.contains("albedo") && materialJson["albedo"].is_string()) {
            texture_paths.insert(materialJson["albedo"].get<std::string>());
        }
    }
  //  std::vector<std::unique_ptr<Texture>> textures;
    std::unordered_map<std::string_view,int> texture_index;
    for(auto & texture_path : texture_paths) {
        textures.push_back(Texture::loadTextureFromFile(device,rootPath.string()+"/"+texture_path));
        texture_index[texture_path] = textures.size() - 1;
    }
    for(auto & materialJson : materialJsons) {
        RTMaterial rtMaterial;
        if(materialJson.contains("albedo") && materialJson["albedo"].is_string()) {
            rtMaterial.texture_id = texture_index[materialJson["albedo"].get<std::string>()];
        }
        else {
            rtMaterial.texture_id = -1;
            rtMaterial.albedo = materialJson["albedo"];
        }
        rtMaterial.bsdf_type = type2RTBSDFTYPE[materialJson["type"].get<std::string>()];
        rtMaterials.push_back(rtMaterial);
        
        GltfMaterial material = InitGltfMaterial();
        material.pbrBaseColorFactor = glm::vec4(rtMaterial.albedo,1);
        material.pbrBaseColorTexture = rtMaterial.texture_id;
        materials.push_back(material);
    }
}

template <typename T>
std::vector<T> getTFromGpuBuffer(Buffer& buffer) {
    Buffer stagingBuffer(buffer.getDevice(), buffer.getSize(), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    CommandBuffer commandBuffer = buffer.getDevice().createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkBufferCopy   copyRegion{0, 0, buffer.getSize()};
    vkCmdCopyBuffer(commandBuffer.getHandle(), buffer.getHandle(), stagingBuffer.getHandle(), 1, &copyRegion);
    g_context->submit(commandBuffer,true);
    return stagingBuffer.getData<T>();
}


void JsonLoader::loadPrimitives() {
    const Json &  primitivesJson = sceneJson["primitives"];

    uint32_t vertexOffset   = 0;
    uint32_t indexOffset    = 0;
  //  uint32_t maxVertexCount = 0;
  //  uint32_t maxIndexCount  = 0;

    std::vector<std::unordered_map<std::string, BufferData>> vertexDatas;
    std::vector<BufferData>                                  indexBuffers;
    //   std::vector<Transform> transforms;
    std::vector<uint32_t> vertexOffsets, indexOffsets;

    vertexAttributes =  {
        {POSITION_ATTRIBUTE_NAME, VertexAttribute{.format = VK_FORMAT_R32G32B32_SFLOAT, .stride = sizeof(glm::vec3)}},
        {NORMAL_ATTRIBUTE_NAME, VertexAttribute{.format = VK_FORMAT_R32G32B32_SFLOAT, .stride = sizeof(glm::vec3)}},
        {TEXCOORD_ATTRIBUTE_NAME, VertexAttribute{.format = VK_FORMAT_R32G32_SFLOAT, .stride = sizeof(glm::vec2)}},
    };

    for (auto& primitiveJson : primitivesJson) {

        {
            if(primitiveJson.contains("emission")) {
                vec3 color = primitiveJson["emission"];
                lights.push_back(SgLight{.type = LIGHT_TYPE::Area,.lightProperties = {.color = color,.prim_index  = static_cast<uint32_t>(primitives.size())}});
            }
        }

        std::unique_ptr<PrimitiveData> primitiveData;
        if(primitiveJson.contains("file")) {
            primitiveData = PrimitiveLoader::loadPrimitive(rootPath.string()+"/"+primitiveJson["file"].get<std::string>());
        }
        else if(primitiveJson.contains("type")) {
            std::string type = primitiveJson["type"];
            primitiveData = PrimitiveLoader::loadPrimitiveFromType(type);
        }
        else {
            LOGE("Primitive must have a type or a file")
        }
        if(primitiveData == nullptr) 
           {
               LOGW("Failed to load primitive");
               continue;
        }
       // auto primitiveData = PrimitiveLoader::loadPrimitive(rootPath.string()+"/"+primitiveJson["file"].get<std::string>());
        //
        uint32_t vertexCount = primitiveData->buffers.at(POSITION_ATTRIBUTE_NAME).size() / sizeof(glm::vec3);
        uint32_t indexCount  = primitiveData->indexs.size();

    //    maxVertexCount = std::max(maxVertexCount, vertexCount);
     //   maxIndexCount  = std::max(maxIndexCount, indexCount);

        

        uint32_t material_index;
        {
             auto &  materialJson = primitiveJson["bsdf"];
            if (materialJson.is_string()) {
                CHECK_RESULT(materialIndexMap.contains(materialJson));
                material_index = materialIndexMap[materialJson.get<std::string>()];
            } else {
                materialJsons.emplace_back(std::move(materialJson));
                material_index = materialJsons.size() - 1;
            }
        }
        auto primitive = std::make_unique<Primitive>(vertexOffset, indexOffset, indexCount, material_index);
        vertexOffsets.push_back(vertexOffset);
        indexOffsets.push_back(indexOffset);


        vertexOffset += vertexCount;
        indexOffset += indexCount;

        indexBuffers.emplace_back(std::move(primitiveData->indexs));
        vertexDatas.push_back(std::move(primitiveData->buffers));

        Transform transform;
        if (primitiveJson.contains("transform")) {
            auto transformJson = primitiveJson["transform"];
            auto position      = GetOptional(transformJson, "position", glm::vec3(0.0f));
            transform.setPosition(position);
            auto rotation = GetOptional(transformJson, "rotation", glm::vec3(0.0f));
            transform.setRotation(math::eulerYZXQuat(rotation));
            auto scale = GetOptional(transformJson, "scale", glm::vec3(1.0f));
            transform.setLocalScale(scale);
        }
        //  transforms.push_back(transform);
        primitive->transform = transform;
        primitives.push_back(std::move(primitive));
    }


    std::unordered_map<std::string,std::unique_ptr<Buffer>> stagingVertexBuffers; 
    auto stagingIndexBuffer  = std::make_unique<Buffer>(device, indexOffset * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

    for (auto& attri : vertexAttributes) {
        auto buffer                    = std::make_unique<Buffer>(device, attri.second.stride * vertexOffset,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT ,
            VMA_MEMORY_USAGE_CPU_TO_GPU);
        // sceneVertexBuffer[attri.first] = std::move(buffer);
        stagingVertexBuffers[attri.first] = std::move(buffer);
    }
    //sceneIndexBuffer = std::make_unique<Buffer>(device, indexOffset * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // auto copyBuffer = [this, &commandBuffer, &stagingVertexBuffers,stagingIndexBuffer](Buffer & srcBuffer,Buffer& dstBuffer, uint32_t size, uint32_t offset) {
    //     VkBufferCopy copy{0, offset, size};
    //     vkCmdCopyBuffer(commandBuffer.getHandle(), srcBuffer.getHandle(), dstBuffer.getHandle(), 1, &copy);  
    // };
    for (uint32_t i = 0; i < primitives.size(); i++) {
        for (auto& [name, bufferData] : vertexDatas[i]) {
            auto vec3_data  = reinterpret_cast<const glm::vec3*>(bufferData.data());
            stagingVertexBuffers[name]->uploadData(bufferData.data(), bufferData.size(),vertexOffsets[i] * vertexAttributes[name].stride);
          //  copyBuffer(*sceneVertexBuffer[name], bufferData.size(), vertexOffsets[i] * vertexAttributes[name].stride);
        }
        stagingIndexBuffer->uploadData(indexBuffers[i].data(), indexBuffers[i].size(),indexOffsets[i] * sizeof(uint32_t));
     //   copyBuffer(*sceneIndexBuffer, indexBuffers[i].size(), indexOffsets[i] * sizeof(uint32_t));
    }

    for (auto& [name, buffer] : stagingVertexBuffers) {
        sceneVertexBuffer[name] = Buffer::FromBuffer(device,commandBuffer, *buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }
    sceneIndexBuffer = Buffer::FromBuffer(device,commandBuffer, *stagingIndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    
    g_context->submit(commandBuffer,true);

   // auto vertex_data = getTFromGpuBuffer<glm::vec3>(*sceneVertexBuffer[POSITION_ATTRIBUTE_NAME]);
   // auto vertex_data1 = getTFromGpuBuffer<glm::vec3>(*(stagingBuffer);


    {
        sceneUniformBuffer = std::make_unique<Buffer>(device, sizeof(PerPrimitiveUniform) * primitives.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        std::vector<PerPrimitiveUniform> uniforms;
        for (uint32_t i = 0; i < primitives.size(); i++) {
            PerPrimitiveUniform uniform;
            uniform.model         = primitives[i]->transform.getLocalToWorldMatrix();
            uniform.modelIT       = glm::transpose(glm::inverse(uniform.model));
            uniform.materialIndex = primitives[i]->materialIndex;
            uniforms.push_back(uniform);
        }
        sceneUniformBuffer->uploadData(uniforms.data(), uniforms.size() * sizeof(PerPrimitiveUniform));
    }
    {
        scenePrimitiveIdBuffer = std::make_unique<Buffer>(device, sizeof(uint32_t) * primitives.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        std::vector<uint32_t> primitiveIds;
        for (uint32_t i = 0; i < primitives.size(); i++) {
            primitiveIds.push_back(i);
        }
        scenePrimitiveIdBuffer->uploadData(primitiveIds.data(), primitiveIds.size() * sizeof(uint32_t));
    }
}
void JsonLoader::Valiation() {
    CHECK_RESULT(primitives.size() > 0);
    CHECK_RESULT(materials.size() > 0);
    CHECK_RESULT(lights.size() > 0);
    CHECK_RESULT(cameras.size() > 0);
    CHECK_RESULT(sceneVertexBuffer.size() > 0);
    CHECK_RESULT(sceneIndexBuffer != nullptr);
    CHECK_RESULT(sceneUniformBuffer != nullptr);
    CHECK_RESULT(scenePrimitiveIdBuffer != nullptr);
}



std::unique_ptr<Scene> Jsonloader::LoadSceneFromGLTFFile(Device& device, const std::string& path,  const SceneLoadingConfig & config) {
    JsonLoader loader(device);
    loader.LoadSceneFromGLTFFile(device, path, config);

    auto scene                = std::make_unique<Scene>();
    scene->primitives         = std::move(loader.primitives);
    scene->textures           = std::move(loader.textures);
    scene->materials          = std::move(loader.materials);
    scene->rtMaterials          = std::move(loader.rtMaterials);
    scene->lights             = std::move(loader.lights);
    scene->cameras            = std::move(loader.cameras);
    scene->vertexAttributes   = std::move(loader.vertexAttributes);
    scene->sceneVertexBuffer  = std::move(loader.sceneVertexBuffer);
    scene->sceneIndexBuffer   = std::move(loader.sceneIndexBuffer);
    
    scene->sceneUniformBuffer = std::move(loader.sceneUniformBuffer);
    scene->indexType          = VK_INDEX_TYPE_UINT32;
    scene->primitiveIdBuffer  = std::move(loader.scenePrimitiveIdBuffer);

    auto vertex_data = getTFromGpuBuffer<glm::vec3>(scene->getVertexBuffer(POSITION_ATTRIBUTE_NAME));
   
    return scene;
}