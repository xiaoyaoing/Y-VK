#include "DDGI.h"

static  std::string kRaygen = "ddgi/raygen.rgen";
static  std::string kClassify = "ddgi/classify.comp";
static  std::string kSampleProbe = "ddgi/sampleProbe.comp";
static  std::string kClosestHit = "Raytracing/PT/closesthit.rchit";
static  std::string kMiss = "Raytracing/PT/miss.rmiss";
static  std::string kMissShadow = "Raytracing/PT/miss_shadow.rmiss";
static  std::string kRayAnyHit = "Raytracing/ray.rahit";

static  std::vector<std::string> GeneratePorbeRays = {
    kRaygen,
    kMiss,
    kMissShadow,
    kClosestHit,
    kRayAnyHit
};

void DDGI::render(RenderGraph& renderGraph) {
    renderGraph.addRaytracingPass(
        "ddgi_probe_trace", 
        [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
            settings.accel = &entry_->tlas;
            settings.shaderPaths = GeneratePorbeRays;
            settings.rTPipelineSettings.dims = {width,height,1};
            settings.rTPipelineSettings.maxDepth = 5;
 
            auto output = renderGraph.createTexture(
                RT_IMAGE_NAME,
                {width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE,VK_FORMAT_R32G32B32A32_SFLOAT}
            );
            builder.writeTexture(output,TextureUsage::STORAGE); 
        }, 
        [&](RenderPassContext& context) {
            auto commandBuffer = context.commandBuffer;

            bindRaytracingResources(commandBuffer);
            renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME));
            renderContext->traceRay(commandBuffer, {width, height, 1}); 
        }
    );
}
