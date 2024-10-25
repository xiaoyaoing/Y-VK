#include "DDGIIntegrator.h"
#include "shaders/Raytracing/ddgi/ddgi_commons.h"
static std::string    kRaygen            = "ddgi/raygen.rgen";
static std::string    kClassify          = "ddgi/classify.comp";
static std::string    kSampleProbe       = "ddgi/sampleProbe.comp";
static std::string    kClosestHit        = "Raytracing/PT/closesthit.rchit";
static std::string    kMiss              = "Raytracing/PT/miss.rmiss";
static std::string    kMissShadow        = "Raytracing/PT/miss_shadow.rmiss";
static std::string    kRayAnyHit         = "Raytracing/ray.rahit";
static std::string    kVisualizeVert     = "ddgi/ddgi_debug.vert";
static std::string    kVisualizeFrag     = "ddgi/ddgi_debug.frag";
static std::string    kRadianceUpdate    = "ddgi/ddgi_update_radiance.comp";
static std::string    kDirectionUpdate   = "ddgi/ddgi_update_direction.comp";
static constexpr uint kSphereVertexCount = 2880;

static std::vector<std::string> GeneratePorbeRays = {
    kRaygen,
    kMiss,
    kMissShadow,
    kClosestHit,
    kRayAnyHit};

struct DDGIIntegrator::DDGIBuffers {
    std::unique_ptr<Buffer> probeRayData;
    std::unique_ptr<Buffer> probeRayCount;
    std::unique_ptr<Buffer> probeOffsets;
    std::unique_ptr<Buffer> uboBuffer;

    std::unique_ptr<SgImage> ddgiIrradiance[2];
    std::unique_ptr<SgImage> ddgiDist[2];
};

struct Probe {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    float     distance;
    float     weight;
};

struct ProbeData {
    std::vector<Probe>     probes;
    std::vector<glm::vec3> probeColors;
    std::vector<float>     probeDistances;
    std::vector<float>     probeWeights;
};

static std::string kProbeDataBufferName = "probeDataBuffer";

void DDGIIntegrator::render(RenderGraph& renderGraph) {

    //Trace rays to generate probe data
    renderGraph.addRaytracingPass(
        "ddgi_probe_trace",
        [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
            settings.accel                       = &entry_->tlas;
            settings.shaderPaths                 = GeneratePorbeRays;
            settings.rTPipelineSettings.dims     = {width, height, 1};
            settings.rTPipelineSettings.maxDepth = 5;

            auto probeRayBuffer = renderGraph.importBuffer(kProbeDataBufferName, buffers->probeRayData.get());
            builder.writeBuffer(probeRayBuffer, BufferUsage::STORAGE);
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;

            bindRaytracingResources(commandBuffer);
            g_context->bindBuffer(0, *buffers->uboBuffer).bindBuffer(6, *buffers->probeRayData).bindBuffer(7, *buffers->probeOffsets);
            uint probeCount = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
            g_context->traceRay(commandBuffer, VkExtent3D{probeCount, uint(ubo.rays_per_probe), 1});
        });

    //Update radiance data

    //update direction and depth data

    uint ping = 0;
    uint pong = 1;

    renderGraph.addComputePass(
        "",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            auto prevRadiance = renderGraph.importTexture("prev_radiance_map", buffers->ddgiIrradiance[ping].get());
            auto radiance     = renderGraph.importTexture("ddgi_radiance_map", buffers->ddgiIrradiance[pong].get());
            builder.writeTexture(radiance, RenderGraphTexture::Usage::STORAGE);
            builder.readTexture(prevRadiance, RenderGraphTexture::Usage::SAMPLEABLE);
            builder.readBuffer(renderGraph.getBlackBoard().getHandle(kProbeDataBufferName), RenderGraphBuffer::Usage::STORAGE);
        },
        [&](RenderPassContext& context) {
            g_context->bindShaders({kRadianceUpdate}).flushAndDispatch(context.commandBuffer, ubo.probe_counts.x * ubo.probe_counts.y, ubo.probe_counts.z, 1);
        });

    renderGraph.addGraphicPass(
        "Visualize ddgi_probe",
        [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
            auto radiance = renderGraph.getBlackBoard().getHandle("ddgi_radiance_map");
            builder.readTexture(radiance, TextureUsage::SAMPLEABLE);

            auto                      output = renderGraph.getBlackBoard().getHandle(RENDER_VIEW_PORT_IMAGE_NAME);
            RenderGraphPassDescriptor desc;
            desc.textures = {output};
            desc.addSubpass({.outputAttachments = {output}});
            builder.declare(desc);
        },
        [&](RenderPassContext& context) {
            g_context->bindShaders({kVisualizeVert, kVisualizeFrag}).getPipelineState().setInputAssemblyState({.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST}).setVertexInputState({});
            g_context->flushAndDraw(context.commandBuffer, kSphereVertexCount);
        });

    // renderGraph.addGraphicPass("Visualize ddgi_probe", [&](RenderGraph::Builder& builder) {
    //     builder.readBuffer(*buffers->probeRayData, AccessType::READ);
    //     builder.readBuffer(*buffers->probeRayCount, AccessType::READ);
    //     builder.readBuffer(*buffers->probeOffsets, AccessType::READ);
    //     builder.readBuffer(*buffers->uboBuffer, AccessType::READ);
    //     builder.writeTexture("ddgi_probe", AccessType::WRITE);
    // }, [&](RenderContext& context) {
    //     auto& commandBuffer = context.commandBuffer;
    //     auto& probeRayData = buffers->probeRayData;
    //     auto& probeRayCount = buffers->probeRayCount;
    //     auto& probeOffsets = buffers->probeOffsets;
    //     auto& ddgiProbe = buffers->ddgiIrradiance[0];
    //     auto& uboBuffer = buffers->uboBuffer;
    //
    //     g_context->bindStorageImage(0, *ddgiProbe);
    //     g_context->bindBuffer(1, *probeRayData);
    //     g_context->bindBuffer(2, *probeRayCount);
    //     g_context->bindBuffer(3, *probeOffsets);
    //     g_context->bindBuffer(4, *uboBuffer);
    //     g_context->dispatch(commandBuffer, glm::ceil(ubo.probe_counts.x / 16.0f), glm::ceil(ubo.probe_counts.y / 16.0f), glm::ceil(ubo.probe_counts.z / 16.0f));
    // });
}
void DDGIIntegrator::init() {
    Integrator::init();
}
void DDGIIntegrator::initScene(RTSceneEntry& entry) {
    Integrator::initScene(entry);
    ubo.probe_counts = (entry.scene->getSceneBBox().max() - entry.scene->getSceneBBox().min()) / config.probe_distance;
    buffers          = new DDGIBuffers();

    uint numProbes         = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
    buffers->probeRayData  = std::make_unique<Buffer>(device, sizeof(DDGIRayData) * numProbes * ubo.rays_per_probe, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeRayCount = std::make_unique<Buffer>(device, sizeof(uint32_t) * numProbes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeOffsets  = std::make_unique<Buffer>(device, sizeof(vec3) * numProbes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

    VkExtent3D imageExtent     = {uint32(ubo.probe_counts.x * ubo.probe_counts.y * 18), uint32(ubo.probe_counts.z * 18), 1};
    buffers->ddgiIrradiance[0] = std::make_unique<SgImage>(device, "ddgi_irradiance0", imageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiIrradiance[1] = std::make_unique<SgImage>(device, "ddgi_irradiance1", imageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiDist[0]       = std::make_unique<SgImage>(device, "ddgi_dist0", imageExtent, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiDist[1]       = std::make_unique<SgImage>(device, "ddgi_dist1", imageExtent, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
}
