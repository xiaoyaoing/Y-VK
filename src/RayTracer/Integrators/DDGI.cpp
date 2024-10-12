#include "DDGI.h"

static constexpr  std::string kRaygen = "ddgi/raygen.rgen";
static constexpr  std::string kClassify = "ddgi/classify.comp";
static constexpr  std::string kSampleProbe = "ddgi/sampleProbe.comp";
static constexpr  std::string kClosestHit =  "Raytracing/PT/closesthit.rchit";
static constexpr  std::string kMiss = "Raytracing/PT/miss.rmiss";
static constexpr  std::string kMissShadow = "Raytracing/PT/miss_shadow.rmiss";
static constexpr  std::string kRayAnyHit = "Raytracing/ray.rahit";

static constexpr  std::vector<std::string> GeneratePorbeRays = {
    kRaygen,kMiss,kMissShadow,kClosestHit,kRayAnyHit
};

class DDGI::Impl {
    friend class DDGI;
    void render(RenderGraph& renderGraph) {
        // Render the graph
        renderGraph.addRaytracingPass(
                "PT pass", [&](RenderGraph::Builder& builder, RaytracingPassSettings& settings) {
                settings.accel = &entry_->tlas;
                settings.pipelineLayout = layout.get();
                settings.rTPipelineSettings.dims = {width,height,1};
                settings.rTPipelineSettings.maxDepth = 5;
    
    
                auto output = renderGraph.createTexture(RT_IMAGE_NAME,{width,height,TextureUsage::STORAGE | TextureUsage::TRANSFER_SRC | TextureUsage::SAMPLEABLE,VK_FORMAT_R32G32B32A32_SFLOAT});
                builder.writeTexture(output,TextureUsage::STORAGE); }, [&](RenderPassContext& context) {
                    bindRaytracingResources(commandBuffer);
    
                    auto pushConstant = toBytes(pcPath);
                    renderContext->bindPushConstants(pushConstant);
                    renderContext->bindImage(0, renderGraph.getBlackBoard().getImageView(RT_IMAGE_NAME));
                    renderContext->traceRay(commandBuffer, {width, height, 1});
    
                    pcPath.frame_num++; });    }
    Impl() {
        
    }
};

 DDGI::DDGI() {
    impl = new Impl();
}

void DDGI::render(RenderGraph& graph) {
    impl->render(graph);
}

