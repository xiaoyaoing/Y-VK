#include "DDGIIntegrator.h"
#include "Common/ResourceCache.h"
#include "shaders/Raytracing/ddgi/ddgi_commons.h"

static ShaderKey      kRaygen             = "Raytracing/ddgi/raygen.rgen";
static ShaderKey      kClassify           = "Raytracing/ddgi/classify.comp";
static ShaderKey      kSampleProbe        = "Raytracing/ddgi/sampleProbe.comp";
static ShaderKey      kClosestHit         = "Raytracing/PT/closesthit.rchit";
static ShaderKey      kMiss               = "Raytracing/PT/miss.rmiss";
static ShaderKey      kMissShadow         = "Raytracing/PT/miss_shadow.rmiss";
static ShaderKey      kRayAnyHit          = "Raytracing/ray.rahit";
static ShaderKey      kVisualizeVert      = "Raytracing/ddgi/ddgi_debug.vert";
static ShaderKey      kVisualizeFrag      = "Raytracing/ddgi/ddgi_debug.frag";
static ShaderKey      kRadianceUpdate     = "Raytracing/ddgi/ddgi_update_radiance.comp";
static ShaderKey      kDirectionUpdate    = "Raytracing/ddgi/ddgi_update_direction.comp";
static ShaderKey      kPrimaryRayGen      = "Raytracing/primary_ray.rgen";
static ShaderKey      kPrimaryRayGenDepth = {"Raytracing/primary_ray.rgen", {"WRITE_DEPTH"}};
static constexpr uint kSphereVertexCount  = 2880;

static ShaderPipelineKey GeneratePorbeRays = {
    kRaygen,
    kMiss,
    kMissShadow,
    kClosestHit,
    kRayAnyHit};

static ShaderPipelineKey PrimaryRayGen = {
    kPrimaryRayGen,
    kMiss,
    kMissShadow,
    kClosestHit,
    kRayAnyHit};

static ShaderPipelineKey PrimaryRayGenDepth = {
    kPrimaryRayGenDepth,
    kMiss,
    kMissShadow,
    kClosestHit,
    kRayAnyHit};

struct DDGIIntegrator::DDGIBuffers {
    std::unique_ptr<Buffer>  probeRayData;
    std::unique_ptr<Buffer>  probeRayCount;
    std::unique_ptr<Buffer>  probeOffsets;
    std::unique_ptr<Buffer>  uboBuffer;
    std::unique_ptr<Buffer>  gbufferBuffer;
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
    if (!entry_->primAreaBuffersInitialized) {
        initLightAreaDistribution(renderGraph);
    }

    renderGraph.setCutUnUsedResources(false);

    renderGraph.addRaytracingPass(
        "Generate Primary Rays", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
            settings.accel = &entry_->tlas;
            settings.shaderPaths = debugDDGI ? PrimaryRayGenDepth : PrimaryRayGen;
            settings.rTPipelineSettings.dims = {width, height, 1};
            builder.writeTexture(RT_IMAGE_NAME, TextureUsage::STORAGE);

            if (debugDDGI) {
                auto depth = renderGraph.createTexture(DEPTH_IMAGE_NAME, {.extent = g_context->getViewPortExtent(), .useage = TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE | TextureUsage::STORAGE});
                builder.writeTexture(depth, TextureUsage::STORAGE);
            }

            settings.rTPipelineSettings.maxDepth = 2; }, [&](RenderPassContext& context) {
            
            auto& commandBuffer = context.commandBuffer;
            bindRaytracingResources(commandBuffer);
            if (debugDDGI)
                g_context->bindImage(1, renderGraph.getBlackBoard().getImageView(DEPTH_IMAGE_NAME));
            g_context->bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME));
            g_context->bindPushConstants(pc_ray);
            g_context->traceRay(commandBuffer, VkExtent3D{width, height, 1}); });

    renderGraph.addRaytracingPass(
        "ddgi_probe_trace", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
            settings.accel = &entry_->tlas;
            settings.shaderPaths = GeneratePorbeRays;
            settings.rTPipelineSettings.dims = {width, height, 1};
            settings.rTPipelineSettings.maxDepth = 5;

            auto probeRayBuffer = renderGraph.importBuffer(kProbeDataBufferName, buffers->probeRayData.get());
            builder.writeBuffer(probeRayBuffer, BufferUsage::STORAGE); }, [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;
            bindRaytracingResources(commandBuffer);
            g_context->bindBuffer(0, *buffers->uboBuffer).bindBuffer(6, *buffers->probeRayData).bindBuffer(7, *buffers->probeOffsets);
            uint probeCount = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
            g_context->bindPushConstants(pc_ray).traceRay(commandBuffer, VkExtent3D{probeCount, uint(ubo.rays_per_probe), 1}); });

    renderGraph.addComputePass(
        "", [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            auto prevRadiance = renderGraph.importTexture("prev_radiance_map", buffers->ddgiIrradiance[ping].get());
            auto radiance = renderGraph.importTexture("ddgi_radiance_map", buffers->ddgiIrradiance[pong].get());
            builder.writeTexture(radiance, RenderGraphTexture::Usage::STORAGE);
            builder.readTexture(prevRadiance, RenderGraphTexture::Usage::SAMPLEABLE);
            builder.readBuffer(renderGraph.getBlackBoard().getHandle(kProbeDataBufferName), RenderGraphBuffer::Usage::READ); }, [&](RenderPassContext& context) {
            auto& sampler = device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);
            g_context->bindImageSampler(0, renderGraph.getBlackBoard().getImageView("prev_radiance_map"), sampler).bindImage(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map")).bindBuffer(6, renderGraph.getBlackBoard().getBuffer(kProbeDataBufferName)).bindBuffer(0, *buffers->uboBuffer);
            g_context->bindShaders({kRadianceUpdate}).flushAndDispatch(context.commandBuffer, ubo.probe_counts.x * ubo.probe_counts.y, ubo.probe_counts.z, 1); });

    if (debugDDGI)
        renderGraph.addGraphicPass(
            "Visualize ddgi_probe", [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                auto radiance = renderGraph.getBlackBoard().getHandle("ddgi_radiance_map");
                builder.readTexture(radiance, TextureUsage::SAMPLEABLE);

                auto output = renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME);
                builder.readAndWriteTexture(output, TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE);
                auto depth = renderGraph.getBlackBoard().getHandle(DEPTH_IMAGE_NAME);
                builder.readAndWriteTexture(depth, TextureUsage::DEPTH_ATTACHMENT);

                RenderGraphPassDescriptor desc;
                desc.textures = {output, depth};
                desc.addSubpass({.outputAttachments = {output, depth}});
                builder.declare(desc); }, [&](RenderPassContext& context) {
                g_context->bindShaders({kVisualizeVert, kVisualizeFrag}).getPipelineState().setVertexInputState({}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
                g_context->bindBuffer(0, *buffers->uboBuffer).bindBuffer(2, *entry_->sceneUboBuffer).bindImageSampler(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map"), device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1));
                uint probeCount = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
                g_context->flushAndDraw(context.commandBuffer, kSphereVertexCount, probeCount, 0, 0); });
}

void DDGIIntegrator::init() {
    Integrator::init();
    gbufferPass = std::make_unique<GBufferPass>();
    gbufferPass->init();
}

void DDGIIntegrator::initScene(RTSceneEntry& entry) {
    Integrator::initScene(entry);
    ubo.probe_counts   = 1.1f * (entry.scene->getSceneBBox().max() - entry.scene->getSceneBBox().min()) / config.probe_distance;
    ubo.rays_per_probe = config.rays_per_probe;
    buffers            = new DDGIBuffers();

    uint numProbes = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;

    buffers->probeRayData  = std::make_unique<Buffer>(device, sizeof(DDGIRayData) * numProbes * ubo.rays_per_probe, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeRayCount = std::make_unique<Buffer>(device, sizeof(uint32_t) * numProbes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeOffsets  = std::make_unique<Buffer>(device, sizeof(vec3) * numProbes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->gbufferBuffer = std::make_unique<Buffer>(device, sizeof(GBuffer) * width * height, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    LOGI("{} num probes", numProbes);

    entry_->sceneDesc.gbuffer_addr       = buffers->gbufferBuffer->getDeviceAddress();
    entry_->sceneDesc.ddgi_ray_data_addr = buffers->probeRayData->getDeviceAddress();
    entry_->sceneDescBuffer->uploadData(&entry_->sceneDesc, sizeof(entry_->sceneDesc));

    VkExtent3D imageExtent   = {uint32(ubo.probe_counts.x * ubo.probe_counts.y * 18), uint32(ubo.probe_counts.z * 18), 1};
    ubo.irradiance_height    = imageExtent.height;
    ubo.irradiance_width     = imageExtent.width;
    ubo.probe_start_position = entry.scene->getSceneBBox().min();

    buffers->uboBuffer = std::make_unique<Buffer>(device, sizeof(DDGIUbo), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, &ubo);

    buffers->ddgiIrradiance[0] = std::make_unique<SgImage>(device, "ddgi_irradiance0", imageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiIrradiance[1] = std::make_unique<SgImage>(device, "ddgi_irradiance1", imageExtent, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiDist[0]       = std::make_unique<SgImage>(device, "ddgi_dist0", imageExtent, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiDist[1]       = std::make_unique<SgImage>(device, "ddgi_dist1", imageExtent, VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_VIEW_TYPE_2D);

    pc_ray.probe_rotation      = glm::mat4(1.0f);
    pc_ray.size_x              = imageExtent.width;
    pc_ray.size_y              = imageExtent.height;
    pc_ray.frame_num           = 0;
    pc_ray.light_num           = mScene->getLights().size();
    pc_ray.enable_sample_bsdf  = 1;
    pc_ray.enable_sample_light = 1;
    pc_ray.max_depth           = 2;
    pc_ray.min_depth           = 0;
    pc_ray.probe_rotation      = glm::mat4(1.0f);

    LOGI("DDGI Integrator initialized");
}

void DDGIIntegrator::onUpdateGUI() {
    Integrator::onUpdateGUI();
    ImGui::Checkbox("Debug DDGI", &debugDDGI);
}
