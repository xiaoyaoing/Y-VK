#include "VisualizeVoxelPass.h"

#include "VxgiCommon.h"
#include "Core/RenderContext.h"
#include "Core/View.h"

struct VisualizeParams {
    glm::mat4  u_viewProj;
    glm::ivec3 u_imageMin;
    int        u_hasPrevClipmapLevel;
    glm::ivec3 u_regionMin;
    int        u_clipmapResolution;
    glm::ivec3 u_prevRegionMin;
    int        u_clipmapLevel;
    glm::ivec3 u_prevRegionMax;
    float      u_voxelSize;
    int        u_hasMultipleFaces;
    int        u_numColorComponents;
    glm::vec2  padding;
};

struct alignas(16) VisualizeFragConstant {
    float     u_borderWidth{0.05f};
    float     u_alpha{1.f};
    glm::vec4 u_borderColor{glm::vec4(0.5f, 0.5f, 0.5f, 1.f)};
};

std::unique_ptr<Primitive> GetVoxelPrimitive(int resolution) {

    std::size_t       vertexCount = std::size_t(resolution * resolution * resolution);
    std::vector<vec3> position;
    position.reserve(vertexCount);
    for (int z = 0; z < resolution; ++z) {
        for (int y = 0; y < resolution; ++y) {
            for (int x = 0; x < resolution; ++x) {
                position.push_back(vec3(x, y, z));
            }
        }
    }

    auto primitive = std::make_unique<Primitive>(0, vertexCount, 0);

    primitive->setVertxAttribute(POSITION_ATTRIBUTE_NAME, VertexAttribute{VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)});
    auto vertex_buffer = std::make_unique<Buffer>(g_context->getDevice(), sizeof(vec3) * vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, position.data());
    primitive->setVertexBuffer(POSITION_ATTRIBUTE_NAME, vertex_buffer);
    primitive->primitiveType = PRIMITIVE_TYPE::E_POINT_LIST;
    return primitive;
}

void VisualizeVoxelPass::init() {
    mPipelineLayout = std::make_unique<PipelineLayout>(
        g_context->getDevice(), std::vector<std::string>{"vxgi/visualization/voxelVisualization.vert", "vxgi/visualization/voxelVisualization.geom", "vxgi/visualization/voxelVisualization.frag"});
    mUniformBuffer  = std::make_unique<Buffer>(g_context->getDevice(), sizeof(VisualizeParams), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
    mVoxelPrimitive = GetVoxelPrimitive(VOXEL_RESOLUTION);
}

void VisualizeVoxelPass::render(RenderGraph& rg) {
}
void VisualizeVoxelPass::visualize3DClipmapGS(RenderGraph& rg, RenderGraphHandle texture, const ClipmapRegion& region, uint32_t clipmapLevel, ClipmapRegion* prevRegion, bool hasMultipleFaces, int numColorComponents) {
    VisualizeParams params;
    params.u_viewProj            = g_manager->fetchPtr<View>("view")->getCamera()->viewProj();
    auto t                       = params.u_viewProj * glm::vec4(g_manager->fetchPtr<View>("view")->getCamera()->getPosition(), 1);
    params.u_imageMin            = region.getMinPosImage(region.extent);
    params.u_regionMin           = region.minCoord;
    params.u_hasPrevClipmapLevel = prevRegion ? 1 : 0;
    if (params.u_hasPrevClipmapLevel) {
        params.u_prevRegionMin = prevRegion->minCoord;
        params.u_prevRegionMax = prevRegion->getMaxCoord();
    }
    params.u_clipmapResolution  = region.extent.x;
    params.u_clipmapLevel       = clipmapLevel;
    params.u_voxelSize          = region.voxelSize;
    params.u_hasMultipleFaces   = hasMultipleFaces ? 1 : 0;
    params.u_numColorComponents = numColorComponents;

    mUniformBuffer->uploadData(&params, sizeof(VisualizeParams));

    rg.addPass(
        "VisualizeVoxelPass", [&](auto& builder, auto& settings) {
        RenderGraphPassDescriptor desc{};
            auto output = rg.getBlackBoard().getHandle(SWAPCHAIN_IMAGE_NAME);
            auto depth  = rg.getBlackBoard().getHandle("depth");
        desc.textures = {texture, output,depth};
        desc.addSubpass({.inputAttachments =  {},.outputAttachments = {output,depth}});
        builder.readTextures({texture,output}).writeTextures({output,depth});    
        builder.declare(desc); }, [texture, this, &rg](RenderPassContext& context) {
            auto& view       = *g_manager->fetchPtr<View>("view");
            g_context->getPipelineState().setPipelineLayout(*mPipelineLayout);

            VisualizeFragConstant fragConstant;
            g_context->bindImageSampler(0, rg.getTexture(texture)->getHwTexture()->getVkImageView(),*g_manager->fetchPtr<Sampler>("radiance_map_sampler")).bindPushConstants(fragConstant)
            .bindPrimitiveGeom(context.commandBuffer,*mVoxelPrimitive)
            .bindBuffer(0, *mUniformBuffer)
            .flushAndDraw(context.commandBuffer, mVoxelPrimitive->vertexCount ); });
}