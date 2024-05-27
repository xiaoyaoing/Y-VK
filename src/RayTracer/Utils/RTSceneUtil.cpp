#include "RTSceneUtil.h"

#include "Common/Distrib.hpp"
#include "Core/RenderContext.h"
#include "Raytracing/commons.h"
#include "Scene/SceneLoader/HDRSampling.h"

#include <numeric>

// static Scene* scene_;

struct RTSceneEntryImpl : public RTSceneEntry {
    RTSceneEntryImpl(Device& device);
    void           initScene(Scene& scene);
    void           initBuffers(Scene& scene);
    void           buildBLAS();
    void           buildTLAS();
    Accel          createAccel(VkAccelerationStructureCreateInfoKHR& accel);
    RTLight        toRTLight(Scene& scene,const SgLight& light);
    Device&        device;
    RenderContext* renderContext{nullptr};
};

RTSceneEntryImpl::RTSceneEntryImpl(Device& device) : device(device) {
    renderContext = g_context;
}

static VkTransformMatrixKHR toVkTransformMatrix(const glm::mat4& matrix) {
    VkTransformMatrixKHR transformMatrix{};
    const auto           tMatrix = glm::transpose(matrix);

    memcpy(transformMatrix.matrix, &tMatrix, sizeof(transformMatrix.matrix));
    return transformMatrix;
}

Accel RTSceneEntryImpl::createAccel(VkAccelerationStructureCreateInfoKHR& accel) {
    Accel result_accel;
    result_accel.buffer = std::make_unique<Buffer>(device, accel.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    accel.buffer        = result_accel.buffer->getHandle();
    // Create the acceleration structure
    VK_CHECK_RESULT(vkCreateAccelerationStructureKHR(device.getHandle(), &accel, nullptr, &result_accel.accel));
    return result_accel;
}

RTLight RTSceneEntryImpl::toRTLight(Scene & scene,const SgLight& light) {
    RTLight rtLight{};

    if (light.type == LIGHT_TYPE::Area) {
        uint32_t prim_idx = light.lightProperties.prim_index;
        rtLight.world_matrix = scene.getPrimitives()[prim_idx]->transform.getLocalToWorldMatrix();
        rtLight.prim_idx     = prim_idx;
        rtLight.L            = light.lightProperties.color;
        rtLight.light_type = RT_LIGHT_TYPE_AREA;
    }
    else if(light.type == LIGHT_TYPE::Point) {
        rtLight.position = light.lightProperties.position;
        rtLight.L        = light.lightProperties.color;
        rtLight.light_type = RT_LIGHT_TYPE_POINT;
    }
    else if(light.type == LIGHT_TYPE::Sky) {
        HDRSampling hdrSampling;
        rtLight.L = light.lightProperties.color;
        rtLight.world_matrix = light.lightProperties.world_matrix;
        rtLight.light_texture_id = light.lightProperties.texture_index;
        rtLight.light_type = RT_LIGHT_TYPE_INFINITE;
        Texture * tex = scene.getTextures().operator[](rtLight.light_texture_id).get();
        auto aceel = hdrSampling.createEnvironmentAccel(reinterpret_cast<const float*>(tex->image->getData().data()),tex->image->getExtent2D());
        infiniteSamplingBuffer = std::make_unique<Buffer>(device, sizeof(EnvAccel) * aceel.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, aceel.data());
        sceneDesc.env_sampling_addr = infiniteSamplingBuffer->getDeviceAddress();
        sceneDesc.envmap_idx = lights.size();
    }

    return rtLight;
}

void RTSceneEntryImpl::initScene(Scene& scene_) {
    scene = &scene_;
    
    indexBuffer  = &scene->getIndexBuffer();
    uvBuffer     = &scene->getVertexBuffer(TEXCOORD_ATTRIBUTE_NAME);
    normalBuffer = &scene->getVertexBuffer(NORMAL_ATTRIBUTE_NAME);
    vertexBuffer = &scene->getVertexBuffer(POSITION_ATTRIBUTE_NAME);

    textures.resize(scene->getTextures().size());
    std::ranges::transform(scene->getTextures().begin(), scene->getTextures().end(), textures.begin(), [](const auto& texture) { return texture.get(); });

    if (!scene->getRTMaterials().empty())
        materials = scene->getRTMaterials();
    else
        for (const auto& material : scene->getGltfMaterials()) {
            RTMaterial rtMaterial{};
            rtMaterial.albedo         = material.pbrBaseColorFactor;
            rtMaterial.texture_id     = material.pbrBaseColorTexture;
            rtMaterial.emissiveFactor = material.emissiveFactor;
            materials.push_back(rtMaterial);
        }

    for (auto& primitive : scene->getPrimitives()) {
        primitives.push_back(RTPrimitive{
            .material_index = primitive->materialIndex,
            .vertex_offset  = primitive->firstVertex,
            .vertex_count   = primitive->vertexCount,
            .index_offset   = primitive->firstIndex,
            .index_count    = primitive->indexCount,
            .light_index    = primitive->lightIndex,
            .world_matrix   = primitive->transform.getLocalToWorldMatrix(),
        });
        if (materials[primitives.back().material_index].emissiveFactor != vec3(0.0f)) {
            scene->addLight(SgLight{.type = LIGHT_TYPE::Area, .lightProperties = {
                                                                 .color      = materials[primitives.back().material_index].emissiveFactor,
                                                                 .prim_index = toUint32(primitives.size()) - 1,
                                                             }});
            primitives.back().light_index = scene->getLights().size() - 1;
        }
    }

   // primitives.resize(1);

    buildBLAS();

    buildTLAS();

    std::unordered_map<uint32_t, std::unique_ptr<Buffer>> primAreaBuffers{};

    struct PC {
        uint32_t index_offset;
        uint32_t index_count;
        uint64_t vertex_address;
        uint64_t index_address;
        mat4     model;
    };

    //  RenderGraph graph(device);

    sceneDesc.envmap_idx = -1;
    for (auto light : scene->getLights()) {
        lights.push_back(toRTLight(*scene,light));
    }
    primitiveMeshBuffer = std::make_unique<Buffer>(device, sizeof(RTPrimitive) * primitives.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, primitives.data());
    materialsBuffer     = std::make_unique<Buffer>(device, sizeof(RTMaterial) * materials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, materials.data());
    rtLightBuffer       = std::make_unique<Buffer>(device, sizeof(RTLight) * lights.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, lights.data());

    sceneUboBuffer = std::make_unique<Buffer>(device, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    sceneDesc.vertex_addr    = vertexBuffer->getDeviceAddress();
        sceneDesc.index_addr     = indexBuffer->getDeviceAddress(),
        sceneDesc.normal_addr    = normalBuffer->getDeviceAddress(),
        sceneDesc.uv_addr        = uvBuffer->getDeviceAddress(),
        sceneDesc.material_addr  = materialsBuffer->getDeviceAddress(),
        sceneDesc.prim_info_addr = primitiveMeshBuffer->getDeviceAddress();
    sceneDescBuffer = std::make_unique<Buffer>(device, sizeof(SceneDesc), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &sceneDesc);
    
}
void RTSceneEntryImpl::initBuffers(Scene& scene) {
    bool     useStagingBuffer   = true;
    uint32_t positionBufferSize = 0;
    uint32_t indexBufferSize    = 0;
    for (const auto& primitive : scene.getPrimitives()) {
        if (primitive->getVertexAttribute(POSITION_ATTRIBUTE_NAME))
            positionBufferSize += primitive->getVertexBuffer(POSITION_ATTRIBUTE_NAME).getSize();
        indexBufferSize += primitive->getIndexBuffer().getSize();
    }
    vertexBuffer = new Buffer(device, positionBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    indexBuffer  = new Buffer(device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    normalBuffer = new Buffer(device, positionBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    const auto uvBufferSize = positionBufferSize * 2 / 3;
    uvBuffer                = new Buffer(device, uvBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    std::unique_ptr<Buffer>             stagingVertexBuffer{nullptr}, stagingIndexBuffer{nullptr};
    static constexpr VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const VkBufferUsageFlags            staging_flags      = useStagingBuffer ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : buffer_usage_flags;

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    std::vector<VkBufferCopy> bufferCopies;
    uint32                    vertexBufferOffset{0}, indexBufferOffset{0}, uvBufferOffset{0}, normalBufferOffset{0};

    uint32 vertexOffset{0}, indexOffset{0}, normalOffset{0};

    auto copyBuffer = [&](Buffer& dstBuffer, const Buffer& srcBuffer, uint32_t& offset) {
        VkBufferCopy copy{.srcOffset = 0, .dstOffset = offset, .size = srcBuffer.getSize()};
        vkCmdCopyBuffer(commandBuffer.getHandle(), srcBuffer.getHandle(), dstBuffer.getHandle(), 1, &copy);
        offset += srcBuffer.getSize();///stride;
    };

    for (auto& primitive : scene.getPrimitives()) {
 
        primitives.emplace_back(RTPrimitive{.material_index = primitive->materialIndex, .vertex_offset = vertexOffset, .vertex_count = primitive->vertexCount, .index_offset = indexOffset, .index_count = primitive->indexCount, .world_matrix = primitive->getTransformMatrix()});

        copyBuffer(*vertexBuffer, primitive->getVertexBuffer(POSITION_ATTRIBUTE_NAME), vertexBufferOffset);
        copyBuffer(*indexBuffer, primitive->getIndexBuffer(), indexBufferOffset);
        copyBuffer(*normalBuffer, primitive->getVertexBuffer(NORMAL_ATTRIBUTE_NAME), normalBufferOffset);
        copyBuffer(*uvBuffer, primitive->getVertexBuffer(TEXCOORD_ATTRIBUTE_NAME), uvBufferOffset);

        vertexOffset += primitive->vertexCount;
        indexOffset += primitive->indexCount;
    }

    renderContext->submit(commandBuffer);
}

void RTSceneEntryImpl::buildBLAS() {
    std::vector<BlasInput> blasInputs;

    // scene->IteratePrimitives([&blasInputs](const Primitive & primitive)
    // {
    //     blasInputs.emplace_back(toVkGeometry(primitive));
    // });
    for (uint32 i = 0; i < primitives.size(); i++) {
        auto& primitive = primitives[i];

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType                                = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat             = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer->getDeviceAddress() + primitive.vertex_offset * sizeof(glm::vec3);
        // accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = scene_->getPrimitives()[i]->vertfexBuffers.at("position")->getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.maxVertex               = primitive.vertex_count;
        accelerationStructureGeometry.geometry.triangles.vertexStride            = sizeof(glm::vec3);
        accelerationStructureGeometry.geometry.triangles.indexType               = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = indexBuffer->getDeviceAddress() + primitive.index_offset * sizeof(uint32_t);
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount  = primitive.index_count / 3;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex     = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;

        blasInputs.emplace_back(BlasInput{{accelerationStructureGeometry}, {accelerationStructureBuildRangeInfo}});
    }

    uint32_t nBlas = toUint32(blasInputs.size());
    // nBlas = 8;

    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometryInfos(nBlas, {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR});
    std::vector<VkAccelerationStructureBuildSizesInfoKHR>    sizeInfos(nBlas, {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR});
    std::vector<Accel>                                       accels(nBlas);
    std::vector<std::unique_ptr<Buffer>>                     scratchBuffers(nBlas);

    VkDeviceSize maxScratchBufferSize = 0;
    for (uint32_t i = 0; i < nBlas; i++) {
        buildGeometryInfos[i].geometryCount = blasInputs[i].geometry.size();
        buildGeometryInfos[i].pGeometries   = blasInputs[i].geometry.data();
        buildGeometryInfos[i].flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildGeometryInfos[i].mode          = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfos[i].type          = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

        std::vector<uint32_t> maxPrimCount(blasInputs[i].range.size());
        for (auto tt = 0; tt < blasInputs[i].range.size(); tt++) {
            maxPrimCount[tt] = blasInputs[i].range[tt].primitiveCount;
        }

        vkGetAccelerationStructureBuildSizesKHR(device.getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfos[i], maxPrimCount.data(), &sizeInfos[i]);
        scratchBuffers[i] = std::make_unique<Buffer>(device, sizeInfos[i].buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    }

    // Buffer scratchBuffer(*device,maxScratchBufferSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);
    // VkDeviceSize scratchAddress = scratchBuffer.getDeviceAddress();

    std::vector<uint32_t> indices;// Indices of the BLAS to create
    VkDeviceSize          batchSize{0};
    VkDeviceSize          batchLimit{256'000'000};// 256 MB

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    for (uint32_t i = 0; i < nBlas; i++) {
        indices.push_back(i);
        batchSize += sizeInfos[i].accelerationStructureSize;
        if (batchSize >= batchLimit || i == nBlas - 1) {

            for (const auto& blasIdx : indices) {
                VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
                createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                createInfo.size = sizeInfos[blasIdx].accelerationStructureSize;// Will be used to allocate memory.

                accels[blasIdx]                                       = createAccel(createInfo);
                buildGeometryInfos[blasIdx].dstAccelerationStructure  = accels[blasIdx].accel;
                buildGeometryInfos[blasIdx].scratchData.deviceAddress = scratchBuffers[blasIdx]->getDeviceAddress();
                // BuildInfo #2 part
                // buildAs[idx].build_info.dstAccelerationStructure = buildAs[idx].as.accel;  // Setting where the build lands
                // buildAs[idx].build_info.scratchData.deviceAddress =
                // All build are using the same scratch buffer
                // Building the bottom-level-acceleration-structure
                const VkAccelerationStructureBuildRangeInfoKHR* range_info = blasInputs[blasIdx].range.data();
                vkCmdBuildAccelerationStructuresKHR(commandBuffer.getHandle(), 1, &buildGeometryInfos[blasIdx], &range_info);
                //break;
            }

            batchSize = 0;
            indices.clear();
        }
    }

    renderContext->submit(commandBuffer);

    blases = std::move(accels);
}

void RTSceneEntryImpl::buildTLAS() {
    auto cmdBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    std::vector<TlasInput> tlasInputs;

    uint primCount = toUint32(primitives.size());
    for (uint32_t i = 0; i < primCount; i++) {
        VkAccelerationStructureInstanceKHR instance{};
        instance.transform                              = toVkTransformMatrix(primitives[i].world_matrix);
        instance.instanceCustomIndex                    = i;
        instance.mask                                   = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference         = blases[i].buffer->getDeviceAddress();
        tlasInputs.emplace_back(instance);
    }

    uint32_t instanceCount = toUint32(tlasInputs.size());

    //Allocate instance Buffer
    auto stagingBuffer = std::make_unique<Buffer>(device, sizeof(TlasInput) * tlasInputs.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, tlasInputs.data());

    auto instanceBuffer = Buffer::FromBuffer(device, cmdBuffer, *stagingBuffer, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR);

    const auto instanceBufferAddress = instanceBuffer->getDeviceAddress();

    VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    vkCmdPipelineBarrier(cmdBuffer.getHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

    VkAccelerationStructureGeometryInstancesDataKHR instances_vk{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
    instances_vk.data.deviceAddress = instanceBufferAddress;

    //Create topAs Geom
    VkAccelerationStructureGeometryKHR topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    topASGeometry.geometryType       = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    topASGeometry.geometry.instances = instances_vk;

    //Get size info of build TLAS
    VkAccelerationStructureBuildGeometryInfoKHR build_info{
        VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    build_info.flags         = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    build_info.geometryCount = 1;
    build_info.pGeometries   = &topASGeometry;
    build_info.mode =
        VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.type                     = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    build_info.srcAccelerationStructure = VK_NULL_HANDLE;

    VkAccelerationStructureBuildSizesInfoKHR size_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    vkGetAccelerationStructureBuildSizesKHR(device.getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info, &instanceCount, &size_info);

    //Create TLAS
    VkAccelerationStructureCreateInfoKHR create_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    create_info.size = size_info.accelerationStructureSize;
    tlas             = createAccel(create_info);

    //Build TLAS
    auto                      scratchBuffer = std::make_unique<Buffer>(device, size_info.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, scratchBuffer->getHandle()};
    VkDeviceAddress           scratchAddress = vkGetBufferDeviceAddress(device.getHandle(), &bufferInfo);

    // Update build information
    build_info.srcAccelerationStructure  = VK_NULL_HANDLE;
    build_info.dstAccelerationStructure  = tlas.accel;
    build_info.scratchData.deviceAddress = scratchAddress;

    // Build Offsets info: n instances
    VkAccelerationStructureBuildRangeInfoKHR        buildOffsetInfo{instanceCount, 0, 0, 0};
    const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;

    // Build the TLAS
    vkCmdBuildAccelerationStructuresKHR(cmdBuffer.getHandle(), 1, &build_info, &pBuildOffsetInfo);

    renderContext->submit(cmdBuffer);
}

std::unique_ptr<RTSceneEntry> RTSceneUtil::convertScene(Device& device, Scene& scene) {
    auto result = std::make_unique<RTSceneEntryImpl>(device);
    result->initScene(scene);
    return result;
}