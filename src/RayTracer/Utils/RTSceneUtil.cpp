#include "RTSceneUtil.h"

#include "Core/RenderContext.h"

static Scene * scene_;

struct RTSceneEntryImpl : public  RTSceneEntry
{
    RTSceneEntryImpl(Device & device);
    void initScene(Scene & scene);
    void buildBLAS();
    void buildTLAS();
    Accel createAccel(VkAccelerationStructureCreateInfoKHR& accel);
    Device & device;
    RenderContext * renderContext{nullptr};
};

RTSceneEntryImpl::RTSceneEntryImpl(Device & device) : device(device)
{
    renderContext = &RenderContext::get();
}


static VkTransformMatrixKHR toVkTransformMatrix(const glm::mat4 & matrix)
{
    VkTransformMatrixKHR transformMatrix{};
    const  auto tMatrix = glm::transpose(matrix);

    memcpy(transformMatrix.matrix, &tMatrix, sizeof(transformMatrix.matrix));
    return transformMatrix;
}

Accel RTSceneEntryImpl::createAccel(VkAccelerationStructureCreateInfoKHR& accel)
{
    Accel result_accel;
    result_accel.buffer = std::make_unique<Buffer>(device,accel.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY);
    accel.buffer = result_accel.buffer->getHandle();
    // Create the acceleration structure
    VK_CHECK_RESULT(vkCreateAccelerationStructureKHR(device.getHandle(), &accel, nullptr, &result_accel.accel));
    return result_accel;
}


void RTSceneEntryImpl::initScene(Scene& scene)
{
    scene_ = &scene;
    bool useStagingBuffer = true;;
    uint32_t positionBufferSize =  0;
    uint32_t indexBufferSize = 0;
    for(const auto & primitive : scene.getPrimitives())
    {
        if(primitive->vertexAttributes.contains(POSITION_ATTRIBUTE_NAME))
            positionBufferSize += primitive->vertexBuffers.at(POSITION_ATTRIBUTE_NAME)->getSize();
        // if(primitive->vertexAttributes.contains(INDEX_ATTRIBUTE_NAME))
        indexBufferSize += primitive->indexBuffer->getSize();
    }
    vertexBuffer = std::make_unique<Buffer>(device, positionBufferSize,
         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    indexBuffer = std::make_unique<Buffer>(device, indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    normalBuffer = std::make_unique<Buffer>(device, positionBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    const auto uvBufferSize = positionBufferSize * 2 / 3;
    uvBuffer = std::make_unique<Buffer>(device, uvBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    std::unique_ptr<Buffer> stagingVertexBuffer{nullptr},stagingIndexBuffer{nullptr};
    static constexpr VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const VkBufferUsageFlags            staging_flags      = useStagingBuffer ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : buffer_usage_flags;

  
    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    std::vector<VkBufferCopy> bufferCopies;
  uint32 vertexBufferOffset{0},indexBufferOffset{0},uvBufferOffset{0},normalBufferOffset{0};

    uint32 vertexOffset{0},indexOffset{0},normalOffset{0};

    auto copyBuffer = [&](Buffer & dstBuffer,Buffer & srcBuffer,uint32_t & offset)
    {
        VkBufferCopy copy{.srcOffset = 0,.dstOffset = offset,.size =   srcBuffer.getSize()};
        vkCmdCopyBuffer(commandBuffer.getHandle(),srcBuffer.getHandle(),dstBuffer.getHandle(),1,&copy);
        offset += srcBuffer.getSize();///stride;
    };
    
    for(auto & primitive : scene.getPrimitives())
    {

         primitives.emplace_back(RTPrimitive{.material_index = primitive->materialIndex,  .vertex_offset = vertexOffset,.vertex_count = primitive->vertexCount,
                                 .index_offset =  indexOffset,.index_count = primitive->indexCount,.world_matrix = primitive->matrix});
        
        copyBuffer(*vertexBuffer,*primitive->vertexBuffers.at(POSITION_ATTRIBUTE_NAME),vertexBufferOffset);
        copyBuffer(*indexBuffer,*primitive->indexBuffer,indexBufferOffset);
        copyBuffer(*normalBuffer,*primitive->vertexBuffers.at(NORMAL_ATTRIBUTE_NAME),normalBufferOffset);

        vertexOffset += primitive->vertexCount;
        indexOffset += primitive->indexCount;
        normalOffset += primitive->vertexCount;
        

        transformBuffers.emplace_back(Buffer{device, sizeof(glm::mat4),
                                                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
                                                            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                                                            | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                                                            VMA_MEMORY_USAGE_CPU_TO_GPU,&primitive->matrix});
        auto & trBuffer = primitive->vertexBuffers.at("transform");
        transformBuffers.push_back(std::move(*trBuffer));
    }
    
    renderContext->submit(commandBuffer);

    buildBLAS();
    buildTLAS();


    std::unordered_map<const Texture *,uint32_t > textureMap; 
    for(const auto & texture : scene.getTextures())
    {
        textures.emplace_back(texture.get());
        textureMap[texture.get()] = toUint32(textures.size() - 1);
    }

    for(const auto & material : scene.getMaterials())
    {
        RTMaterial rtMaterial{};
        rtMaterial.albedo =  material.baseColorFactor;
        if(material.textures.contains("baseColorTexture"))
            rtMaterial.texture_id = textureMap.at(&material.textures.at("baseColorTexture"));
        rtMaterial.emissiveFactor = material.emissiveFactor;
        materials.push_back(rtMaterial);
    }

    for(uint32_t i = 0; i< primitives.size() ; i++)
    {
        const auto & primitive = primitives[i];
        if(materials[primitive.material_index].emissiveFactor != glm::vec3(0.0f))
        {
            RTLight light{};
         //   light.light_flags = 0;
            light.prim_idx = i;
            light.world_matrix = primitive.world_matrix;
            light.L = materials[primitive.material_index].emissiveFactor;
            lights.emplace_back(light);
        }
    }

     primitiveMeshBuffer =  std::make_unique<Buffer>(device, sizeof(RTPrimitive) * primitives.size(),
                                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                                            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                                            VMA_MEMORY_USAGE_CPU_TO_GPU,primitives.data());
    materialsBuffer = std::make_unique<Buffer>(device, sizeof(RTMaterial) * materials.size(),
                                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                                            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
                                                            VMA_MEMORY_USAGE_CPU_TO_GPU,materials.data());
    rtLightBuffer = std::make_unique<Buffer>(device, sizeof(RTLight) * lights.size(),
                                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                                                            VMA_MEMORY_USAGE_CPU_TO_GPU,lights.data());
    
}


void RTSceneEntryImpl::buildBLAS()
{
     std::vector<BlasInput> blasInputs;

    // scene->IteratePrimitives([&blasInputs](const Primitive & primitive)
    // {
    //     blasInputs.emplace_back(toVkGeometry(primitive));
    // });
    for(uint32 i = 0 ; i < primitives.size() ; i++)
    {
        auto & primitive = primitives[i];
        
        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
       // accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = vertexBuffer->getDeviceAddress() + primitive.vertex_offset * sizeof(glm::vec3) ;
        // accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = scene_->getPrimitives()[i]->vertfexBuffers.at("position")->getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.maxVertex = primitive.vertex_count;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(glm::vec3);
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = indexBuffer->getDeviceAddress() + primitive.index_offset * sizeof(uint32_t);
     //   accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = scene_->getPrimitives()[i]->indexBuffer->getDeviceAddress();
        // accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = transformBuffers[i].getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount =  primitive.index_count/3;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;

        blasInputs.emplace_back(BlasInput{{accelerationStructureGeometry},{accelerationStructureBuildRangeInfo}});

    }

    uint32_t nBlas  = toUint32(blasInputs.size());
    // nBlas = 8;
    
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometryInfos(nBlas,{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR});
    std::vector<VkAccelerationStructureBuildSizesInfoKHR> sizeInfos(nBlas,{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR});
    std::vector<Accel> accels(nBlas);
    std::vector<std::unique_ptr<Buffer>> scratchBuffers(nBlas);

    VkDeviceSize maxScratchBufferSize = 0;
    for(uint32_t i =0 ; i < nBlas ; i++)
    {
        buildGeometryInfos[i].geometryCount = blasInputs[i].geometry.size();
        buildGeometryInfos[i].pGeometries = blasInputs[i].geometry.data();
        buildGeometryInfos[i].flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        buildGeometryInfos[i].mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildGeometryInfos[i].type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;


        std::vector<uint32_t> maxPrimCount(blasInputs[i].range.size());
        for(auto tt = 0 ; tt < blasInputs[i].range.size() ; tt++)
        {
            maxPrimCount[tt] = blasInputs[i].range[tt].primitiveCount;
        }
        
        vkGetAccelerationStructureBuildSizesKHR(device.getHandle(),VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,&buildGeometryInfos[i],maxPrimCount.data(),&sizeInfos[i]);
        scratchBuffers[i] = std::make_unique<Buffer>(device,sizeInfos[i].buildScratchSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);
        
        
    }


    // Buffer scratchBuffer(*device,maxScratchBufferSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);
    // VkDeviceSize scratchAddress = scratchBuffer.getDeviceAddress();

    
    std::vector<uint32_t> indices;	// Indices of the BLAS to create
    VkDeviceSize batchSize{0};
    VkDeviceSize batchLimit{256'000'000};  // 256 MB

    auto commandBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    for(uint32_t i = 0 ; i < nBlas ; i++)
    {
        indices.push_back(i);
        batchSize += sizeInfos[i].accelerationStructureSize;
        if(batchSize >= batchLimit || i == nBlas - 1)
        {
            
            for(const auto & blasIdx : indices)
            {
                VkAccelerationStructureCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
                createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                createInfo.size = sizeInfos[blasIdx].accelerationStructureSize;	 // Will be used to allocate memory.

                accels[blasIdx] = createAccel(createInfo);
                buildGeometryInfos[blasIdx].dstAccelerationStructure = accels[blasIdx].accel;
                buildGeometryInfos[blasIdx].scratchData.deviceAddress = scratchBuffers[blasIdx]->getDeviceAddress();
                // BuildInfo #2 part
                // buildAs[idx].build_info.dstAccelerationStructure = buildAs[idx].as.accel;  // Setting where the build lands
                // buildAs[idx].build_info.scratchData.deviceAddress =
                  // All build are using the same scratch buffer
                // Building the bottom-level-acceleration-structure
               const VkAccelerationStructureBuildRangeInfoKHR* range_info =  blasInputs[blasIdx].range.data();
                vkCmdBuildAccelerationStructuresKHR(commandBuffer.getHandle(), 1, &buildGeometryInfos[blasIdx], &range_info );
                //break;
            }   
            
            batchSize = 0;
            indices.clear();
        }
    }

    renderContext->submit(commandBuffer);

    blases = std::move(accels);
}

void RTSceneEntryImpl::buildTLAS()
{
   auto cmdBuffer = device.createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,true);
    
   uint32_t primIdx = 0 ;
   std::vector<TlasInput> tlasInputs; 
   for(uint32_t i = 0 ; i < primitives.size() ; i++)
   {
       VkAccelerationStructureInstanceKHR instance{};
       instance.transform = toVkTransformMatrix(primitives[i].world_matrix);
       instance.instanceCustomIndex = i;
       instance.mask = 0xFF;
       instance.instanceShaderBindingTableRecordOffset = 0;
       instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
       instance.accelerationStructureReference = blases[i].buffer->getDeviceAddress();
       tlasInputs.emplace_back(instance);
   }
   
   uint32_t instanceCount = toUint32(tlasInputs.size());

   //Allocate instance Buffer 
   Buffer instanceBuffer(device,sizeof(TlasInput) * tlasInputs.size(),
   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,VMA_MEMORY_USAGE_CPU_TO_GPU,tlasInputs.data()); 
   const auto  instanceBufferAddress = instanceBuffer.getDeviceAddress();
   

   VkAccelerationStructureGeometryInstancesDataKHR instances_vk{
   		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
   	instances_vk.data.deviceAddress = instanceBufferAddress;
   
    //Create topAs Geom 
   	VkAccelerationStructureGeometryKHR topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
   	topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
   	topASGeometry.geometry.instances = instances_vk;
   
   
   //Get size info of build TLAS
   VkAccelerationStructureBuildGeometryInfoKHR build_info{
   		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
   	build_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
   	build_info.geometryCount = 1;
   	build_info.pGeometries = &topASGeometry;
   	build_info.mode =
   		 VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
   	build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
   	build_info.srcAccelerationStructure = VK_NULL_HANDLE;
   
   	VkAccelerationStructureBuildSizesInfoKHR size_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
   	vkGetAccelerationStructureBuildSizesKHR(device.getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info,
   											&instanceCount, &size_info);
   			
   	//Create TLAS										
   	VkAccelerationStructureCreateInfoKHR create_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    		create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    		create_info.size = size_info.accelerationStructureSize;
    tlas = createAccel(create_info);
    	
    
    //Build TLAS
    Buffer scratchBuffer(device,size_info.buildScratchSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);	
    VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, scratchBuffer.getHandle()};
        	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(device.getHandle(), &bufferInfo);
        
        // Update build information
    build_info.srcAccelerationStructure =  VK_NULL_HANDLE;
    build_info.dstAccelerationStructure = tlas.accel;
    build_info.scratchData.deviceAddress = scratchAddress;
        
        // Build Offsets info: n instances
    VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo{instanceCount, 0, 0, 0};
    const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;
    
        // Build the TLAS
    vkCmdBuildAccelerationStructuresKHR(cmdBuffer.getHandle(), 1, &build_info, &pBuildOffsetInfo);
    
    renderContext->submit(cmdBuffer);
}

std::unique_ptr<RTSceneEntry>  RTSceneUtil::convertScene(Device & device,Scene& scene)
{
    auto result =  std::make_unique<RTSceneEntryImpl>(device);
    result->initScene(scene);
    return result;
}
