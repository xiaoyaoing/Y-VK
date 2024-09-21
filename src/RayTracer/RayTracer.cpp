//
// Created by pc on 2023/12/1.
//
#include "Scene/RuntimeSceneManager.h"

#include "RayTracer.h"

#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Core/Shader/GlslCompiler.h"
#include "Integrators/PathIntegrator.h"
#include "Integrators/RestirIntegrator.h"
#include "Integrators/SimpleIntegrator.h"
#include "PostProcess/PostProcess.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "Scene/SceneLoader/gltfloader.h"


RayTracer::RayTracer(const RayTracerSettings& settings):Application("Real time Ray tracer",1920,1080) {
    addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
    addDeviceExtension(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
    addDeviceExtension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

    //Application::g_App = this;
}

void RayTracer::drawFrame(RenderGraph& renderGraph) {

    sceneUbo.projInverse = camera->projInverse();
    sceneUbo.viewInverse = camera->viewInverse();
    sceneUbo.view        = camera->view();
    sceneUbo.proj        = camera->proj();
    sceneUbo.prev_view   = lastFrameSceneUbo.view;
    sceneUbo.prev_proj   = lastFrameSceneUbo.proj;
    rtSceneEntry->sceneUboBuffer->uploadData(&sceneUbo, sizeof(sceneUbo));

    lastFrameSceneUbo = sceneUbo;
    integrators[currentIntegrator]->render(renderGraph);
    // postProcess->render(renderGraph);

    renderGraph.addImageCopyPass(renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME), renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME));
}
void RayTracer::onSceneLoaded() {
    Application::onSceneLoaded();
    rtSceneEntry = RTSceneUtil::convertScene(*device, *scene);
    for (auto& integrator : integrators) {
        integrator.second->initScene(*rtSceneEntry);
        integrator.second->init();
    }
}

void RayTracer::prepare() {
    Application::prepare();
    GlslCompiler::setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_5);
    GlslCompiler::forceRecompile = true;

    integrators["path"]   = std::make_unique<PathIntegrator>(*device);
    integrators["restir"] = std::make_unique<RestirIntegrator>(*device);
    integratorNames       = {"path", "restir"};
    currentIntegrator = "resitr";

    sceneLoadingConfig = {.requiredVertexAttribute = {POSITION_ATTRIBUTE_NAME, INDEX_ATTRIBUTE_NAME, NORMAL_ATTRIBUTE_NAME, TEXCOORD_ATTRIBUTE_NAME},
                          .indexType               = VK_INDEX_TYPE_UINT32,
                          .bufferAddressAble       = true,
                          .bufferForAccel          = true,
                          .bufferForStorage        = true,
                          .sceneScale              = glm::vec3(1.f)};
 loadScene(FileUtils::getResourcePath("kitchen/scene.json"));
 // loadScene("E:/code/vk-raytracing-demo/resources/test-ball/scene.json");
 //   loadScene("E:/code/vk-raytracing-demo/resources/classroom/scene.json");
}

void RayTracer::onUpdateGUI() {
    Application::onUpdateGUI();

    int itemCurrent = 0;
    for (int i = 0; i < integratorNames.size(); i++) {
        if (integratorNames[i] == currentIntegrator) {
            itemCurrent = i;
            break;
        }
    }
    std::vector<const char*> integratorNamesCStr;
    for (auto& integratorName : integratorNames) {
        integratorNamesCStr.push_back(integratorName.data());
    }
    ImGui::Combo("Integrators", &itemCurrent, integratorNamesCStr.data(), integratorNames.size());

    currentIntegrator = integratorNames[itemCurrent];

    integrators[currentIntegrator]->onUpdateGUI();
    // postProcess->updateGui();
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
// 		LUMEN_TRACE(" RT BLAS: reducing from: %u to: %u = %u (%2.2f%s smaller) /n", as_total_size, compact_size,
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

int main() {
    RayTracer rayTracer({});
    rayTracer.prepare();
    rayTracer.mainloop();
}