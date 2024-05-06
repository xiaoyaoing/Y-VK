#include "Integrator.h"

#include "Scene/Compoments/Camera.h"
#include "../Utils/RTSceneUtil.h"

Integrator::Integrator(Device& device) : renderContext(g_context), device(device) {
    // init();
    // storageImage = std::make_unique<SgImage>(device,"",VkExtent3D{width,height,1},VK_FORMAT_B8G8R8A8_UNORM,VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,VMA_MEMORY_USAGE_GPU_ONLY,VK_IMAGE_VIEW_TYPE_2D);
}

void Integrator::initScene(Scene& scene) {
    auto sceneEntry = RTSceneUtil::convertScene(device, scene);

    //  return;
    vertexBuffer = std::move(sceneEntry->vertexBuffer);
    normalBuffer = std::move(sceneEntry->normalBuffer);
    uvBuffer     = std::move(sceneEntry->uvBuffer);
    indexBuffer  = std::move(sceneEntry->indexBuffer);

    materialsBuffer     = std::move(sceneEntry->materialsBuffer);
    primitiveMeshBuffer = std::move(sceneEntry->primitiveMeshBuffer);
    transformBuffers    = std::move(sceneEntry->transformBuffers);
    rtLightBuffer       = std::move(sceneEntry->rtLightBuffer);

    blases     = std::move(sceneEntry->blases);
    tlas       = std::move(sceneEntry->tlas);
    lights     = std::move(sceneEntry->lights);
    primitives = std::move(sceneEntry->primitives);
    materials  = std::move(sceneEntry->materials);

    mScene = &scene;

    sceneUboBuffer = std::make_unique<Buffer>(device, sizeof(SceneUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &sceneUbo);

    camera        = scene.getCameras()[0];
    camera->flipY = false;

    width  = renderContext->getViewPortExtent().width;
    height = renderContext->getViewPortExtent().height;
}

void Integrator::init(Scene& scene_) {

    initScene(scene_);
    computePrimAreaLayout = std::make_unique<PipelineLayout>(device, std::vector<std::string>{"Raytracing/compute_triangle_area.comp"});
}

void Integrator::updateGui() {
}

void Integrator::destroy() {
}

void Integrator::update() {
}

void Integrator::buildBLAS() {
    std::vector<BlasInput> blasInputs;

    // scene->IteratePrimitives([&blasInputs](const Primitive & primitive)
    // {
    //     blasInputs.emplace_back(toVkGeometry(primitive));
    // });
    for (uint32 i = 0; i < primitives.size(); i++) {
        auto& primitive = primitives[i];

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType                                          = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType                                   = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat                = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress    = vertexBuffer->getDeviceAddress() + primitive.vertex_offset * sizeof(glm::vec3);
        accelerationStructureGeometry.geometry.triangles.maxVertex                   = primitive.vertex_count;
        accelerationStructureGeometry.geometry.triangles.vertexStride                = sizeof(glm::vec3);
        accelerationStructureGeometry.geometry.triangles.indexType                   = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress     = indexBuffer->getDeviceAddress() + primitive.index_offset * sizeof(uint32_t);
        accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = transformBuffers[i].getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress   = nullptr;

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

void Integrator::buildTLAS() {
    auto cmdBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    uint32_t               primIdx = 0;
    std::vector<TlasInput> tlasInputs;
    for (uint32_t i = 0; i < primitives.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
        //   instance.transform = toVkTransformMatrix(primitives[i].worldMatrix);
        instance.instanceCustomIndex                    = 0;
        instance.mask                                   = 0xFF;
        instance.instanceShaderBindingTableRecordOffset = 0;
        instance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference         = blases[i].buffer->getDeviceAddress();
        tlasInputs.emplace_back(instance);
    }

    uint32_t instanceCount = toUint32(tlasInputs.size());

    //Allocate instance Buffer
    Buffer     instanceBuffer(device, sizeof(TlasInput) * tlasInputs.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU, tlasInputs.data());
    const auto instanceBufferAddress = instanceBuffer.getDeviceAddress();

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
    Buffer                    scratchBuffer(device, size_info.buildScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, scratchBuffer.getHandle()};
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

Accel Integrator::createAccel(VkAccelerationStructureCreateInfoKHR& accel) {
    Accel result_accel;
    result_accel.buffer = std::make_unique<Buffer>(device, accel.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    accel.buffer        = result_accel.buffer->getHandle();
    // Create the acceleration structure
    VK_CHECK_RESULT(vkCreateAccelerationStructureKHR(device.getHandle(), &accel, nullptr, &result_accel.accel));
    return result_accel;
}

Integrator::~Integrator() {
}

void Integrator::bindRaytracingResources(CommandBuffer& commandBuffer)

{
    sceneUbo.projInverse = camera->projInverse();
    sceneUbo.viewInverse = camera->viewInverse();
    sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    g_context->bindAcceleration(0, tlas).bindBuffer(2, *sceneUboBuffer, 0, sizeof(sceneUbo)).bindBuffer(3, *sceneDescBuffer).bindBuffer(4, *rtLightBuffer);
    uint32_t arrayElement = 0;
    for (const auto& texture : mScene->getTextures()) {
        g_context->bindImageSampler(6, texture->getImage().getVkImageView(), texture->getSampler(), 0, arrayElement++);
    }
}
