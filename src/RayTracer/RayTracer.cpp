//
// Created by pc on 2023/12/1.
//

#include "RayTracer.h"

#include "Common/ResourceCache.h"
#include "Core/Shader/GlslCompiler.h"

struct BlasInput
{
    std::vector<VkAccelerationStructureGeometryKHR> geometry;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> range;
};


using TlasInput  = VkAccelerationStructureInstanceKHR;


static VkTransformMatrixKHR toVkTransformMatrix(const glm::mat4 & matrix)
{
    VkTransformMatrixKHR transformMatrix{};
    const  auto tMatrix = glm::transpose(matrix);

    memcpy(transformMatrix.matrix, &tMatrix, sizeof(transformMatrix.matrix));
    return transformMatrix;
}

static BlasInput toVkGeometry(const Primitive & primitive)
{
   
        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData.deviceAddress = primitive.vertexBuffers.at("position")->getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(glm::vec3);
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData.deviceAddress = primitive.indexBuffer->getDeviceAddress();
        accelerationStructureGeometry.geometry.triangles.transformData.deviceAddress = 0;
        accelerationStructureGeometry.geometry.triangles.transformData.hostAddress = nullptr;

        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = primitive.indexCount  / 3;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;

        return {.geometry = {accelerationStructureGeometry},.range = {accelerationStructureBuildRangeInfo}};
        // accelerationStructureGeometry.geometry.triangles.transformData = transformBufferDeviceAddress;
    
}

static TlasInput toVkInstance(const Primitive & primitive,VkDeviceAddress blasAddress)
{
    VkAccelerationStructureInstanceKHR instance{};
    instance.transform = toVkTransformMatrix(primitive.matrix);
    instance.instanceCustomIndex = 0;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instance.accelerationStructureReference = blasAddress;
    return instance;
}

Accel RayTracer::createAccel(VkAccelerationStructureCreateInfoKHR& accel)
{
    Accel result_accel;
    // Allocating the buffer to hold the acceleration structure
    Buffer buffer(*device,accel.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY );
    
    // Setting the buffer
    result_accel.buffer = std::make_unique<Buffer>(*device,accel.size, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                  VMA_MEMORY_USAGE_GPU_ONLY);
    accel.buffer = result_accel.buffer->getHandle();
    // Create the acceleration structure
    vkCreateAccelerationStructureKHR(device->getHandle(), &accel, nullptr, &result_accel.accel);
    return result_accel;
}

VkDeviceAddress RayTracer::getAccelerationStructureDeviceAddress(uint32_t primIdx)
{
    VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
            nullptr,blases[primIdx].accel};
    return vkGetAccelerationStructureDeviceAddressKHR(device->getHandle(), &deviceAddressInfo);
}

RayTracer::RayTracer(const RayTracerSettings& settings)
{
        addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
        addDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
        addDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
        addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
}

void RayTracer::drawFrame(RenderGraph& renderGraph, CommandBuffer& commandBuffer)
{
    renderContext->getPipelineState().setPipelineType(PIPELINE_TYPE::E_RAY_TRACING).setPipelineLayout(*layout).setRtPassSettings({.maxDepth = 5,.dims = {100,100,100}});
    renderContext->bindPipelineLayout(*layout).bindAcceleration(0,tlas,0,0);
    renderContext->flush(commandBuffer);
    renderContext->traceRay(commandBuffer);
    renderContext->submit(commandBuffer,fence);
    // vkCmdTraceRaysKHR()
}

void RayTracer::prepare()
{
    Application::prepare();
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);

    //
    // vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
    // vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
    // vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
    // vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
    // vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
    // vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
    // vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
    // vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
    // vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
    // vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));

    std::vector<Shader> shaders= {
        Shader(*device,FileUtils::getShaderPath("Raytracing/raygen.rgen")),
        Shader{*device,FileUtils::getShaderPath("Raytracing/miss.rmiss")},
        Shader{*device,FileUtils::getShaderPath("Raytracing/shadow.rmiss")},
        Shader(*device,FileUtils::getShaderPath("Raytracing/closesthit.rchit"))
    };
    layout = &device->getResourceCache().requestPipelineLayout(shaders);

  //  scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("sponza/Sponza01.gltf"));
    scene = GltfLoading::LoadSceneFromGLTFFile(*device, FileUtils::getResourcePath("cornell-box/cornellBox.gltf"));


}

void RayTracer::buildBLAS()
{
    std::vector<BlasInput> blasInputs;

    scene->IteratePrimitives([&blasInputs](const Primitive & primitive)
    {
        blasInputs.emplace_back(toVkGeometry(primitive));
    });

    uint32_t nBlas  = blasInputs.size();
    
    std::vector<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometryInfos(nBlas,{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR});
    std::vector<VkAccelerationStructureBuildSizesInfoKHR> sizeInfos;
    std::vector<Accel> accels;

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
        
        vkGetAccelerationStructureBuildSizesKHR(device->getHandle(),VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,&buildGeometryInfos[i],maxPrimCount.data(),&sizeInfos[i]);

        
        maxScratchBufferSize = std::max(maxScratchBufferSize,sizeInfos[i].buildScratchSize);
    }


    Buffer scratchBuffer(*device,maxScratchBufferSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);
    VkDeviceSize scratchAddress = scratchBuffer.getDeviceAddress();

    
    std::vector<uint32_t> indices;	// Indices of the BLAS to create
    VkDeviceSize batchSize{0};
    VkDeviceSize batchLimit{256'000'000};  // 256 MB

    auto commandBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
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
                buildGeometryInfos[blasIdx].scratchData.deviceAddress = scratchAddress;
                // BuildInfo #2 part
                // buildAs[idx].build_info.dstAccelerationStructure = buildAs[idx].as.accel;  // Setting where the build lands
                // buildAs[idx].build_info.scratchData.deviceAddress =
                  // All build are using the same scratch buffer
                // Building the bottom-level-acceleration-structure
               const VkAccelerationStructureBuildRangeInfoKHR* range_info =  blasInputs[blasIdx].range.data();
                vkCmdBuildAccelerationStructuresKHR(commandBuffer.getHandle(), 1, &buildGeometryInfos[blasIdx], &range_info );
            }   
            
            batchSize = 0;
            indices.clear();
        }
    }

     blases = std::move(accels);
} 



// void RayTracer::buildBLAS()
// {
//     // scene->IteratePrimitives([](const Primitive & primitive)
//     // {
//     //     
//     // });
//      {
// 	uint32_t nb_blas = static_cast<uint32_t>(input.size());
// 	VkDeviceSize as_total_size{0};	   // Memory size of all allocated BLAS
// 	uint32_t nb_compactions{0};		   // Nb of BLAS requesting compaction
// 	VkDeviceSize max_scratch_size{0};  // Largest scratch size
//
// 	// Preparing the information for the acceleration build commands.
// 	std::vector<BuildAccelerationStructure> buildAs(nb_blas);
// 	for (uint32_t idx = 0; idx < nb_blas; idx++) {
// 		// Filling partially the VkAccelerationStructureBuildGeometryInfoKHR for
// 		// querying the build sizes. Other information will be filled in the
// 		// createBlas (see #2)
// 		buildAs[idx].build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
// 		buildAs[idx].build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
// 		buildAs[idx].build_info.flags = input[idx].flags | flags;
// 		buildAs[idx].build_info.geometryCount = static_cast<uint32_t>(input[idx].as_geom.size());
// 		buildAs[idx].build_info.pGeometries = input[idx].as_geom.data();
//
// 		// Build range information
// 		buildAs[idx].range_info = input[idx].as_build_offset_info.data();
//
// 		// Finding sizes to create acceleration structures and scratch
// 		std::vector<uint32_t> maxPrimCount(input[idx].as_build_offset_info.size());
// 		for (auto tt = 0; tt < input[idx].as_build_offset_info.size(); tt++) {
// 			maxPrimCount[tt] = input[idx].as_build_offset_info[tt].primitiveCount;	// Number of primitives/triangles
// 		}
// 		vkGetAccelerationStructureBuildSizesKHR(ctx.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
// 												&buildAs[idx].build_info, maxPrimCount.data(), &buildAs[idx].size_info);
//
// 		// Extra info
// 		as_total_size += buildAs[idx].size_info.accelerationStructureSize;
// 		max_scratch_size = std::max(max_scratch_size, buildAs[idx].size_info.buildScratchSize);
// 		nb_compactions +=
// 			has_flag(buildAs[idx].build_info.flags, VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR);
// 	}
//
// 	// Allocate the scratch buffers holding the temporary data of the
// 	// acceleration structure builder
// 	Buffer scratch_buffer;
// 	scratch_buffer.create(&ctx, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
// 						  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, max_scratch_size);
// 	VkBufferDeviceAddressInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, scratch_buffer.handle};
// 	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(ctx.device, &buffer_info);
//
// 	// Allocate a query pool for storing the needed size for every BLAS
// 	// compaction.
// 	VkQueryPool queryPool{VK_NULL_HANDLE};
// 	if (nb_compactions > 0)	 // Is compaction requested?
// 	{
// 		assert(nb_compactions == nb_blas);	// Don't allow mix of on/off compaction
// 		VkQueryPoolCreateInfo qpci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
// 		qpci.queryCount = nb_blas;
// 		qpci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
// 		vkCreateQueryPool(ctx.device, &qpci, nullptr, &queryPool);
// 	}
// 	// Batching creation/compaction of BLAS to allow staying in restricted
// 	// amount of memory
// 	std::vector<uint32_t> indices;	// Indices of the BLAS to create
// 	VkDeviceSize batchSize{0};
// 	VkDeviceSize batchLimit{256'000'000};  // 256 MB
// 	for (uint32_t idx = 0; idx < nb_blas; idx++) {
// 		indices.push_back(idx);
// 		batchSize += buildAs[idx].size_info.accelerationStructureSize;
// 		// Over the limit or last BLAS element
// 		if (batchSize >= batchLimit || idx == nb_blas - 1) {
// 			CommandBuffer cmdBuf(&ctx, true, 0, QueueType::GFX);
// 			cmd_create_blas(cmdBuf.handle, indices, buildAs, scratchAddress, queryPool);
// 			cmdBuf.submit();
// 			if (queryPool) {
// 				cmd_compact_blas(cmdBuf.handle, indices, buildAs, queryPool);
// 				cmdBuf.submit();
// 				// Destroy the non-compacted version
// 				for (auto i : indices) {
// 					vkDestroyAccelerationStructureKHR(ctx.device, buildAs[i].cleanup_as.accel, nullptr);
// 					buildAs[i].cleanup_as.buffer.destroy();
// 				}
// 			}
// 			// Reset
// 			batchSize = 0;
// 			indices.clear();
// 		}
// 	}
//
// 	// Logging reduction
// 	if (queryPool) {
// 		VkDeviceSize compact_size =
// 			std::accumulate(buildAs.begin(), buildAs.end(), 0ULL,
// 							[](const auto& a, const auto& b) { return a + b.size_info.accelerationStructureSize; });
// 		LUMEN_TRACE(" RT BLAS: reducing from: %u to: %u = %u (%2.2f%s smaller) \n", as_total_size, compact_size,
// 					as_total_size - compact_size, (as_total_size - compact_size) / float(as_total_size) * 100.f, "%");
// 	}
//
// 	// Keeping all the created acceleration structures
// 	for (auto& b : buildAs) {
// 		blases.emplace_back(b.as);
// 	}
// 	// Clean up
// 	vkDestroyQueryPool(ctx.device, queryPool, nullptr);
// 	scratch_buffer.destroy();
// }
// }


void RayTracer::buildTLAS()
{
    auto cmdBuffer = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY,true);
    
   uint32_t primIdx = 0 ;
   std::vector<TlasInput> tlasInputs; 
   scene->IteratePrimitives([&tlasInputs,this,&primIdx](const Primitive & primitive)
   {
       tlasInputs.emplace_back(toVkInstance(primitive,getAccelerationStructureDeviceAddress(primIdx++)));
   });
   
   uint32_t instanceCount = tlasInputs.size();

   //Allocate instance Buffer 
   Buffer instanceBuffer(*device,sizeof(TlasInput) * tlasInputs.size(),
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
   	vkGetAccelerationStructureBuildSizesKHR(device->getHandle(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &build_info,
   											&instanceCount, &size_info);
   			
   	//Create TLAS										
   	VkAccelerationStructureCreateInfoKHR create_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
    		create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    		create_info.size = size_info.accelerationStructureSize;
    tlas = createAccel(create_info);
    	
    
    //Build TLAS
    Buffer scratchBuffer(*device,size_info.buildScratchSize,VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_ONLY);	
    VkBufferDeviceAddressInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, scratchBuffer.getHandle()};
        	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(device->getHandle(), &bufferInfo);
        
        // Update build information
        build_info.srcAccelerationStructure =  VK_NULL_HANDLE;
        build_info.dstAccelerationStructure = tlas.accel;
        build_info.scratchData.deviceAddress = scratchAddress;
    
        // Build Offsets info: n instances
        VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo{instanceCount, 0, 0, 0};
        const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;
    
        // Build the TLAS
        vkCmdBuildAccelerationStructuresKHR(cmdBuffer.getHandle(), 1, &build_info, &pBuildOffsetInfo);									
}


int main()
{
    RayTracer rayTracer({});
    rayTracer.prepare();
    rayTracer.mainloop();
}
