// #include "SimpleIntegrator.h"
//
// #include "Common/ResourceCache.h"
// #include "Scene/Compoments/Camera.h"
//
// void SimpleIntegrator::render(RenderGraph& renderGraph) {
//     auto& commandBuffer = renderContext->getGraphicCommandBuffer();
//
//     renderGraph.addRaytracingPass(
//         "PT pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
//         settings.accel = &tlas;
//         settings.pipelineLayout = layout;
//         settings.rTPipelineSettings.dims = {width,height,1};
//         settings.rTPipelineSettings.maxDepth = 5;
//         
//         auto output = renderGraph.createTexture("RT output",{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC});
//         builder.writeTexture(output,TextureUsage::STORAGE);
//         renderGraph.getBlackBoard().put("RT",output); }, [&](RenderPassContext& context) {
//         auto buffer = renderContext->allocateBuffer(sizeof(cameraUbo),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
//         cameraUbo.projInverse = glm::inverse(camera->proj());
//         cameraUbo.viewInverse = glm::inverse(camera->view());
//         buffer.buffer->uploadData(&cameraUbo,sizeof(cameraUbo));
//         renderContext->bindBuffer(2,*buffer.buffer,0,sizeof(cameraUbo));
//         renderContext->bindImage(1,renderGraph.getBlackBoard().getImageView("RT"));
//         renderContext->traceRay(commandBuffer,{width,height,1}); });
// }
//
// void SimpleIntegrator::initScene(Scene& scene) {
//     Integrator::initScene(scene);
//
//     SceneDesc desc{
//         .vertex_addr    = vertexBuffer->getDeviceAddress(),
//         .index_addr     = indexBuffer->getDeviceAddress(),
//         .normal_addr    = normalBuffer->getDeviceAddress(),
//         .uv_addr        = uvBuffer->getDeviceAddress(),
//         .material_addr  = materialsBuffer->getDeviceAddress(),
//         .prim_info_addr = primitiveMeshBuffer->getDeviceAddress(),
//     };
//     sceneDescBuffer = std::make_unique<Buffer>(device, sizeof(SceneDesc), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &desc);
// }
//
// SimpleIntegrator::SimpleIntegrator(Device& device) : Integrator(device) {
//     layout = new PipelineLayout(device, {"Raytracing/khr_ray_tracing_basic/raygen.rgen", "Raytracing/khr_ray_tracing_basic/closesthit.rchit", "Raytracing/khr_ray_tracing_basic/miss.rmiss"});
// }
