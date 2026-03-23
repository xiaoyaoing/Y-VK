#include "ClipmapUpdatePolicy.h"
#include "VxgiCommon.h"
#include "imgui.h"
#include "Common/Log.h"
#include "Common/VkCommon.h"
#include "Core/Buffer.h"
#include "Core/RenderContext.h"

#include <ext/matrix_clip_space.hpp>
#include <ext/matrix_transform.hpp>

struct VxgiContext::VxgiContextImpl {
    Scene*              scene;
    std::vector<BBox>   bboxes;
    std::vector<ClipmapRegion> * clipmapRegions = nullptr;
    ClipmapUpdatePolicy clipmapUpdatePolicy;
    bool useConservativeRasterization;
    VxgiConfig config;
    
    std::vector<std::unique_ptr<Buffer>> voxelProjectionBuffers;
    std::vector<bool> curFrameBufferUpdated;
};

VxgiContext::VxgiContext() {
    mImpl = new VxgiContextImpl();
    // mImpl->voxelProjectionBuffers.resize(CLIP_MAP_LEVEL_COUNT,nullptr);
    mImpl->curFrameBufferUpdated.resize(CLIP_MAP_LEVEL_COUNT,false);
}

VxgiContext& VxgiContext::getInstance() {
    static VxgiContext instance;
    return instance;
}

std::vector<BBox>& VxgiContext::getBBoxes() {
    return getInstance().mImpl->bboxes;
}
bool VxgiContext::UseConservativeRasterization() {
    return getInstance().mImpl->useConservativeRasterization;
}
VxgiConfig& VxgiContext::getConfig() {
    return getInstance().mImpl->config;
}

void VxgiContext::Gui() {
    ImGui::Checkbox("Use Conservative Rasterization", &VxgiContext::getConfig().useConservativeRasterization);
    ImGui::Checkbox("Use Down Sample", &VxgiContext::getConfig().useDownSample);
    ImGui::Checkbox("Use MSAA", &VxgiContext::getConfig().useMsaa);
}


std::vector<ClipmapRegion>& VxgiContext::getClipmapRegions() {
    if (getInstance().mImpl->clipmapRegions == nullptr) {
       LOGE("Clipmap regions are not set")
    }
    return *getInstance().mImpl->clipmapRegions;
}
void VxgiContext::setClipmapRegions(std::vector<ClipmapRegion>& regions) {
    getInstance().mImpl->clipmapRegions = &regions;
}

ClipmapUpdatePolicy& VxgiContext::getClipmapUpdatePolicy() {
    return getInstance().mImpl->clipmapUpdatePolicy;
}

struct ViewPortMatrix {
    glm::mat4 uViewProj[3];
    glm::mat4 uViewProjIt[3];
};


 Buffer* VxgiContext::GetVoxelProjectionBuffer(int clipmapLevel) {
    if (getInstance().mImpl->curFrameBufferUpdated[clipmapLevel]) {
       return getInstance().mImpl->voxelProjectionBuffers[clipmapLevel].get();
    }
    ViewPortMatrix viewProj;

    auto&           region          = getClipmapRegions()[clipmapLevel];
    const glm::vec3 regionGlobal    = glm::vec3(region.extent) * region.voxelSize;
    const glm::vec3 minCornerGlobal = glm::vec3(region.minCoord) * region.voxelSize;
    const glm::vec3 eye             = minCornerGlobal + glm::vec3(0.0f, 0.0f, regionGlobal.z);

    viewProj.uViewProj[0] = glm::ortho(-regionGlobal.z, regionGlobal.z, -regionGlobal.y, regionGlobal.y, 0.1f, regionGlobal.x) *
                            glm::lookAt(eye, eye + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    viewProj.uViewProj[1] = glm::ortho(-regionGlobal.x, regionGlobal.x, -regionGlobal.z, regionGlobal.z, 0.1f, regionGlobal.y) *
                            glm::lookAt(eye, eye + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    viewProj.uViewProj[2] = glm::ortho(-regionGlobal.x, regionGlobal.x, -regionGlobal.y, regionGlobal.y, 0.1f, regionGlobal.z) *
                            glm::lookAt(minCornerGlobal, minCornerGlobal + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    for (int i = 0; i < 3; i++) {
       // viewProj.uViewProj[i]   = proj[i] * viewProj.uViewProj[i];
       viewProj.uViewProjIt[i] = glm::inverse(viewProj.uViewProj[i]);
    }
    if (getInstance().mImpl->voxelProjectionBuffers.empty()) {
       for (int i = 0; i < CLIP_MAP_LEVEL_COUNT; i++) {
           getInstance().mImpl->voxelProjectionBuffers.push_back(std::make_unique<Buffer>(g_context->getDevice(), sizeof(ViewPortMatrix), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
       }
    }
    getInstance().mImpl->voxelProjectionBuffers[clipmapLevel]->uploadData(&viewProj, sizeof(ViewPortMatrix));
    getInstance().mImpl->curFrameBufferUpdated[clipmapLevel] = true;
    return getInstance().mImpl->voxelProjectionBuffers[clipmapLevel].get();
}
void VxgiContext::SetVoxelzationViewPortPipelineState(CommandBuffer & commandBuffer,glm::uvec3 viewportSize) {
     g_context->getPipelineState().setViewportState({.viewportCount = 3, .scissorCount = 3});
     g_context->getPipelineState().setDepthStencilState({.depthTestEnable = false, .depthWriteEnable = false});
     g_context->getPipelineState().setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
     std::vector<VkViewport> viewports{
         vkCommon::initializers::viewport(float(viewportSize.x), float(viewportSize.y), 0.0f, 1.0f, false),
         vkCommon::initializers::viewport(float(viewportSize.z), float(viewportSize.x), 0.0f, 1.0f, false),
         vkCommon::initializers::viewport(float(viewportSize.x), float(viewportSize.y), 0.0f, 1.0f, false)};
     std::vector<VkRect2D> scissors{
            {{0, 0}, {viewportSize.z, viewportSize.y}},
            {{0, 0}, {viewportSize.x, viewportSize.z}},
            {{0, 0}, {viewportSize.x, viewportSize.y}}};
     commandBuffer.setViewport(0, viewports);
     commandBuffer.setScissor(0, scissors);
     if (VxgiContext::getConfig().useConservativeRasterization) {
         g_context->getPipelineState().enableConservativeRasterization(g_context->getDevice().getPhysicalDevice());
     }
     if (VxgiContext::getConfig().useMsaa) {
         g_context->getPipelineState().setMultisampleState({.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT});
     }
}
void VxgiContext::OnFrameBegin() {
     std::fill(getInstance().mImpl->curFrameBufferUpdated.begin(), getInstance().mImpl->curFrameBufferUpdated.end(), false);
}
