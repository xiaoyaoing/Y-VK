#include "DDGIIntegrator.h"
#include "Common/ResourceCache.h"
#include "Scene/SceneLoader/SceneLoaderInterface.h"
#include "shaders/Raytracing/ddgi/ddgi_commons.h"

static ShaderKey kRaygen             = "Raytracing/ddgi/raygen.rgen";
static ShaderKey kClassify           = "Raytracing/ddgi/classify.comp";
static ShaderKey kSampleProbe        = "Raytracing/ddgi/sample_probe.comp";
static ShaderKey kClosestHit         = "Raytracing/PT/closesthit.rchit";
static ShaderKey kMiss               = "Raytracing/PT/miss.rmiss";
static ShaderKey kMissShadow         = "Raytracing/PT/miss_shadow.rmiss";
static ShaderKey kRayAnyHit          = "Raytracing/ray.rahit";
static ShaderKey kVisualizeVert      = "Raytracing/ddgi/ddgi_debug.vert";
static ShaderKey kVisualizeFrag      = "Raytracing/ddgi/ddgi_debug.frag";
static ShaderKey kRadianceUpdate     = {"Raytracing/ddgi/ddgi_update_radiance.comp", {"UPDATE_RADIANCE"}};
static ShaderKey kDepthUpdate        = {"Raytracing/ddgi/ddgi_update_radiance.comp"};
static ShaderKey kPrimaryRayGen      = "Raytracing/primary_ray.rgen";
static ShaderKey kPrimaryRayGenDepth = {"Raytracing/primary_ray.rgen", {"WRITE_DEPTH"}};
static ShaderKey kShadowMapVert      = "shadows/shadowMap.vert";
static ShaderKey kShadowMapFrag      = "shadows/shadowMap.frag";

static constexpr uint kSphereVertexCount = 2880;

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

    if (useRTGBuffer) {
        renderGraph.addRaytracingPass(
            "Generate Primary Rays",
            [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
                settings.accel                   = &entry_->tlas;
                settings.shaderPaths             = PrimaryRayGen;
                settings.rTPipelineSettings.dims = {width, height, 1};
                builder.writeTexture(RT_IMAGE_NAME, TextureUsage::STORAGE);

                settings.rTPipelineSettings.maxDepth = 2;
            },
            [&](RenderPassContext& context) {
                auto& commandBuffer = context.commandBuffer;
                bindRaytracingResources(commandBuffer);
                g_context->bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME));
                g_context->bindPushConstants(getPC());
                g_context->traceRay(commandBuffer, VkExtent3D{width, height, 1});
            });
    } else {
        auto gbufferBuffer = renderGraph.importBuffer("gbufferBuffer", buffers->gbufferBuffer.get());
        shadowMapPass->render(renderGraph);
        gbufferPass->renderToBuffer(renderGraph, gbufferBuffer, renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME));
    }

    auto radiance       = renderGraph.importTexture("ddgi_radiance_map", buffers->ddgiIrradiance[pong].get());
    auto depth_ddgi_map = renderGraph.importTexture("ddgi_depth_map", buffers->ddgiDist[pong].get());

    renderGraph.addRaytracingPass(
        "ddgi_probe_trace",
        [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
            settings.accel                       = &entry_->tlas;
            settings.shaderPaths                 = GeneratePorbeRays;
            settings.rTPipelineSettings.dims     = {width, height, 1};
            settings.rTPipelineSettings.maxDepth = 5;

            auto probeRayBuffer = renderGraph.importBuffer(kProbeDataBufferName, buffers->probeRayData.get());
            builder.writeBuffer(probeRayBuffer, BufferUsage::STORAGE);
            builder.readTexture(radiance, RenderGraphTexture::Usage::SAMPLEABLE).readTexture(depth_ddgi_map, RenderGraphTexture::Usage::SAMPLEABLE);
        },
        [&](RenderPassContext& context) {
            auto& commandBuffer = context.commandBuffer;
            bindRaytracingResources(commandBuffer);
            auto& sampler = device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);
            g_context->bindImageSampler(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map"), sampler)
                .bindImageSampler(1, renderGraph.getBlackBoard().getImageView("ddgi_depth_map"), sampler);
            g_context->bindBuffer(0, *buffers->uboBuffer)
                .bindBuffer(6, *buffers->probeRayData)
                .bindBuffer(7, *buffers->probeOffsets);
            uint probeCount = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
            g_context->bindPushConstants(getPC())
                .traceRay(commandBuffer, VkExtent3D{probeCount, uint(ubo.rays_per_probe), 1});
        });

    renderGraph.addComputePass(
        "ddgi_update_radiance",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(radiance, RenderGraphTexture::Usage::STORAGE);
            builder.readBuffer(renderGraph.getBlackBoard().getHandle(kProbeDataBufferName), RenderGraphBuffer::Usage::READ);
        },
        [&](RenderPassContext& context) {
            g_context->bindImage(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map"))
                .bindBuffer(6, renderGraph.getBlackBoard().getBuffer(kProbeDataBufferName))
                .bindBuffer(0, *buffers->uboBuffer);
            g_context->bindShaders({kRadianceUpdate})
                .bindPushConstants(getPC())
                .flushAndDispatch(context.commandBuffer, ubo.probe_counts.x * ubo.probe_counts.y, ubo.probe_counts.z, 1);
        });

    renderGraph.addComputePass(
        "ddgi_update_depth",
        [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
            builder.writeTexture(depth_ddgi_map, RenderGraphTexture::Usage::STORAGE);
            builder.readBuffer(renderGraph.getBlackBoard().getHandle(kProbeDataBufferName), RenderGraphBuffer::Usage::READ);
        },
        [&](RenderPassContext& context) {
            g_context->bindImage(0, renderGraph.getBlackBoard().getImageView("ddgi_depth_map"))
                .bindBuffer(6, renderGraph.getBlackBoard().getBuffer(kProbeDataBufferName))
                .bindBuffer(0, *buffers->uboBuffer);
            g_context->bindShaders({kDepthUpdate})
                .bindPushConstants(getPC())
                .flushAndDispatch(context.commandBuffer, ubo.probe_counts.x * ubo.probe_counts.y, ubo.probe_counts.z, 1);
        });

    if (showIndirect)
        renderGraph.addComputePass(
            "ddgi_indirect_lighting",
            [&](RenderGraph::Builder& builder, ComputePassSettings& settings) {
                builder.readTexture("ddgi_radiance_map", RenderGraphTexture::Usage::SAMPLEABLE);
                builder.readTexture("ddgi_depth_map", RenderGraphTexture::Usage::SAMPLEABLE);
                builder.writeTexture(RT_IMAGE_NAME, RenderGraphTexture::Usage::STORAGE);
            },
            [&](RenderPassContext& context) {
                auto& sampler = device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1);
                bindRaytracingResources(context.commandBuffer);

                g_context->bindImageSampler(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map"), sampler)
                    .bindImageSampler(1, renderGraph.getBlackBoard().getImageView("ddgi_depth_map"), sampler)
                    .bindBuffer(3, *entry_->sceneDescBuffer)
                    .bindBuffer(0, *buffers->uboBuffer)
                    .bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME))
                    .bindPushConstants(getPC());
                g_context->bindShaders({kSampleProbe})
                    .flushAndDispatch(context.commandBuffer, (width + 15) / 16, (height + 15) / 16, 1);
            });

    if (debugDDGI) {
        if (!renderGraph.getBlackBoard().contains(DEPTH_IMAGE_NAME))
            renderGraph.addGraphicPass(
                "Only Depth Pass",
                [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                    auto depth = renderGraph.createTexture(
                        DEPTH_IMAGE_NAME,
                        {.extent = g_context->getViewPortExtent(),
                         .useage = TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE | TextureUsage::STORAGE,
                         .format = VK_FORMAT_D32_SFLOAT});
                    builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);
                    RenderGraphPassDescriptor desc;
                    desc.textures = {depth};
                    desc.addSubpass({.outputAttachments = {depth}});
                    builder.declare(desc);
                },
                [&](RenderPassContext& context) {
                    g_context->bindShaders({kShadowMapVert, kShadowMapFrag}).bindPushConstants(camera->viewProj());
                    g_manager->fetchPtr<View>("view")->bindViewGeom(context.commandBuffer).drawPrimitives(context.commandBuffer);
                });

        renderGraph.addGraphicPass(
            "Visualize ddgi_probe",
            [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                auto radiance = renderGraph.getBlackBoard().getHandle("ddgi_radiance_map");
                builder.readTexture(radiance, TextureUsage::SAMPLEABLE);

                auto output = renderGraph.getBlackBoard().getHandle(RT_IMAGE_NAME);
                builder.readAndWriteTexture(output, TextureUsage::COLOR_ATTACHMENT | TextureUsage::SAMPLEABLE);
                auto depth = renderGraph.getBlackBoard().getHandle(DEPTH_IMAGE_NAME);
                builder.readAndWriteTexture(depth, TextureUsage::DEPTH_ATTACHMENT);

                RenderGraphPassDescriptor desc;
                desc.textures = {output, depth};
                desc.addSubpass({.outputAttachments = {output, depth}});
                builder.declare(desc);
            },
            [&](RenderPassContext& context) {
                g_context->bindShaders({kVisualizeVert, kVisualizeFrag})
                    .getPipelineState()
                    .setVertexInputState({})
                    .setRasterizationState({.cullMode = VK_CULL_MODE_BACK_BIT})
                    .setDepthStencilState({.depthTestEnable = true, .depthWriteEnable = true});
                g_context->bindBuffer(0, *buffers->uboBuffer)
                    .bindBuffer(2, *entry_->sceneUboBuffer)
                    .bindImageSampler(0, renderGraph.getBlackBoard().getImageView("ddgi_radiance_map"), device.getResourceCache().requestSampler(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_FILTER_LINEAR, 1));
                uint probeCount = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
                // g_context->bindPrimitiveGeom(context.commandBuffer,*spherePrimitive).flushAndDrawIndexed(context.commandBuffer, spherePrimitive->indexCount, probeCount, 0, 0);
                g_context->flushAndDraw(context.commandBuffer, kSphereVertexCount, probeCount, 0, 0);
            });
    }
}

void DDGIIntegrator::init() {
    Integrator::init();
    gbufferPass = std::make_unique<GBufferPass>();
    gbufferPass->init();
    shadowMapPass = std::make_unique<ShadowMapPass>();
    shadowMapPass->init();
    spherePrimitive = SceneLoaderInterface::loadSpecifyTypePrimitive(device, "sphere");
}

void DDGIIntegrator::initScene(RTSceneEntry& entry) {
    Integrator::initScene(entry);
    ubo.rays_per_probe       = config.rays_per_probe;
    ubo.depth_sharpness      = 50.f;
    ubo.normal_bias          = config.normal_bias;
    getPC().ddgi_normal_bias = config.normal_bias;
    ubo.view_bias            = 0.1f;
    ubo.backface_ratio       = 0.1f;
    ubo.min_frontface_dist   = 0.1f;

    ubo.probe_counts = config.probe_counts;
    ubo.probe_distance       = 1.1f * (entry.scene->getSceneBBox().max() - entry.scene->getSceneBBox().min()) / vec3(ubo.probe_counts);
    ubo.max_distance         = 1.5f * ubo.probe_distance.length();

    buffers = new DDGIBuffers();

    uint numProbes = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;

    if (numProbes < 10 || numProbes > 64 * 64 * 64) {
        vec3 probeDistance= 1.1f * (entry_->scene->getSceneBBox().max() - entry_->scene->getSceneBBox().min()) / 64;
        ubo.probe_counts   = 1.1f * abs(entry_->scene->getSceneBBox().max() - entry_->scene->getSceneBBox().min()) / probeDistance;
        numProbes          = ubo.probe_counts.x * ubo.probe_counts.y * ubo.probe_counts.z;
        ubo.probe_distance = probeDistance;
        ubo.max_distance   = 1.5f * probeDistance.length();
    }

    LOGI("probe counts: {} {} {}", ubo.probe_counts.x, ubo.probe_counts.y, ubo.probe_counts.z);
    buffers->probeRayData = std::make_unique<Buffer>(
        device,
        sizeof(DDGIRayData) * numProbes * ubo.rays_per_probe,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeRayCount = std::make_unique<Buffer>(
        device,
        sizeof(uint32_t) * numProbes,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->probeOffsets = std::make_unique<Buffer>(
        device,
        sizeof(vec3) * numProbes,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
    buffers->gbufferBuffer = std::make_unique<Buffer>(
        device,
        sizeof(GBuffer) * width * height,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
    LOGI("{} num probes", numProbes);

    entry_->sceneDesc.gbuffer_addr       = buffers->gbufferBuffer->getDeviceAddress();
    entry_->sceneDesc.ddgi_ray_data_addr = buffers->probeRayData->getDeviceAddress();
    entry_->sceneDescBuffer->uploadData(&entry_->sceneDesc, sizeof(entry_->sceneDesc));

    VkExtent3D irradianceImageExtent = {uint32(ubo.probe_counts.x * ubo.probe_counts.y * config.irradiance_texel_count), uint32(ubo.probe_counts.z * config.irradiance_texel_count), 1};
    ubo.irradiance_height            = irradianceImageExtent.height;
    ubo.irradiance_width             = irradianceImageExtent.width;
    ubo.probe_start_position         = entry.scene->getSceneBBox().min() + 0.5f * (entry.scene->getSceneBBox().max() - entry.scene->getSceneBBox().min()) / vec3(ubo.probe_counts);

    VkExtent3D depthImageExtent = {uint32(ubo.probe_counts.x * ubo.probe_counts.y * config.distance_texel_count), uint32(ubo.probe_counts.z * config.distance_texel_count), 1};
    ubo.depth_height            = depthImageExtent.height;
    ubo.depth_width             = depthImageExtent.width;

    buffers->uboBuffer = std::make_unique<Buffer>(
        device,
        sizeof(DDGIUbo),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        &ubo);

    buffers->ddgiIrradiance[0] = std::make_unique<SgImage>(
        device,
        "ddgi_irradiance0",
        irradianceImageExtent,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VK_IMAGE_VIEW_TYPE_2D);
    buffers->ddgiDist[0] = std::make_unique<SgImage>(
        device,
        "ddgi_dist0",
        depthImageExtent,
        VK_FORMAT_R16G16_SFLOAT,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        VK_IMAGE_VIEW_TYPE_2D);

    getPC().probe_rotation      = glm::mat4(1.0f);
    getPC().size_x              = width;
    getPC().size_y              = height;
    getPC().frame_num           = 0;
    getPC().light_num           = mScene->getLights().size();
    getPC().enable_sample_bsdf  = 1;
    getPC().enable_sample_light = 1;
    getPC().probe_rotation      = glm::mat4(1.0f);
    getPC().first_frame         = 1;
    getPC().ddgi_show_direct    = 0;
    getPC().ddgi_indirect_scale = 1.0f;

    useRTGBuffer = config.use_rt_gbuffer;
    debugDDGI    = config.debug_ddgi;

    LOGI("DDGI Integrator initialized");
}
DDGIIntegrator::DDGIIntegrator(Device& device, DDGIConfig config) : Integrator(device) {
    this->config = config;
}

void DDGIIntegrator::onUpdateGUI() {
    Integrator::onUpdateGUI();
    ImGui::Checkbox("Debug DDGI", &debugDDGI);
    ImGui::Checkbox("Show indirect light", &showIndirect);
    ImGui::Checkbox("Show direct light", reinterpret_cast<bool*>(&getPC().ddgi_show_direct));
    ImGui::Checkbox("Warp Border", reinterpret_cast<bool*>(&getPC().wrap_border));
    ImGui::SliderFloat("indirect scale", &getPC().ddgi_indirect_scale, 0.0f, 10.0f);
    ImGui::SliderFloat("normal bias", &getPC().ddgi_normal_bias, 0.0f, 1.0f);
    ImGui::Checkbox("Use RT GBuffer", &useRTGBuffer);
}
