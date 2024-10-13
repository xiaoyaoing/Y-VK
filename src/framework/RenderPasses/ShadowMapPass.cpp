#include "ShadowMapPass.h"

#include "Common/ResourceCache.h"
#include "Common/VkCommon.h"
#include "Core/RenderContext.h"
#include "Core/View.h"
#include "Core/math.h"
#include "RenderGraph/RenderGraph.h"
#include "Scene/Scene.h"

static DirectionalLightShadowDesc getLightMVP(const glm::vec3& lightPos, const glm::vec3& lightDir) {
    // glm::mat4 lightView       = glm::lookAt(lightPos, lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
    // float aspect = (float)g_context->getViewPortExtent().width / (float)g_context->getViewPortExtent().height;
    // float near = 1.0f;
    // float far = 96.f;
    // glm::mat4 shadowProj = glm::perspective(glm::radians(45.f), aspect, near, far);
    // // return shadowProj * lightView;

    auto _origin	= lightPos;
    auto _direction	= glm::normalize(lightDir);
    constexpr glm::vec3 kOrthoHalfResolution{ 16.0f, 16.0f, 16.0f };
    glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    if (math::nearEq(std::abs(glm::dot(lightDir, up)), 1.0f))
        up = glm::vec3(0.0f, 0.0f, 1.0f);
    auto view	= glm::lookAtLH(_origin, _origin + _direction, up);
    auto proj	= glm::orthoLH_ZO(
        -kOrthoHalfResolution.x, kOrthoHalfResolution.x,
        -kOrthoHalfResolution.y, kOrthoHalfResolution.y,
                0.1f,                    30.0f
    );
    DirectionalLightShadowDesc desc;
    desc.proj = proj;
    desc.view = view;
    desc.zNear = 0.1f;
    desc.zFar = 30.0f;
    // desc.proj = shadowProj;
    // desc.view = lightView;
    return desc;
}

ShadowMapPass::ShadowMapPass():mSampler(g_context->getDevice().getResourceCache().requestSampler()) {
}

struct ViewProj {
    glm::mat4 view;
    glm::mat4 proj;
};

void ShadowMapPass::render(RenderGraph& rg) {
   auto view = g_manager->fetchPtr<View>("view");
   auto &  lights  = view->getLights();
    
    for(int i = 0; i < lights.size(); i++) {
        if (lights[i].type == LIGHT_TYPE::Directional) {
            if(!lights[i].lightProperties.use_shadow) {
                lights[i].lightProperties.shadow_index = -1;
                g_manager->getView()->setLightDirty(true);
                continue;
            }
            //todo string format
            auto shadowName = std::format("ShadowMapLight{}", i);
            rg.addGraphicPass(
                "ShadowMap",
                [&](RenderGraph::Builder& builder, GraphicPassSettings& settings) {
                    auto depth = rg.createTexture(shadowName, {.extent = VkExtent2D{4096,4096},
                .useage = TextureUsage::SUBPASS_INPUT | TextureUsage::DEPTH_ATTACHMENT | TextureUsage::SAMPLEABLE
                                                   });
                builder.declare(RenderGraphPassDescriptor({depth}, {.outputAttachments = {depth}}));
                builder.writeTexture(depth, TextureUsage::DEPTH_ATTACHMENT);
                },
                
                [&,light = &lights[i],name = shadowName](RenderPassContext& context) {
                    g_context->getPipelineState().setPipelineLayout(*mPipelineLayout).setDepthStencilState({.depthCompareOp = VK_COMPARE_OP_LESS}).setRasterizationState({.cullMode = VK_CULL_MODE_NONE});
                    auto desc = getLightMVP(light->lightProperties.position,light->lightProperties.direction);
                    auto mvp = desc.proj * desc.view;
                    g_context->bindPushConstants(mvp);
                    light->lightProperties.shadow_index = g_manager->getView()->bindTexture(rg.getBlackBoard().getImageView(name), mSampler);
                    light->lightProperties.shadow_matrix = mvp;
                    light->lightProperties.shadow_desc = desc;
                    context.commandBuffer.setViewport(0,{vkCommon::initializers::viewport(float(4096), float(4096), 0.0f, 1.0f, false)});
                    context.commandBuffer.setScissor(0,{vkCommon::initializers::rect2D(4096, 4096, 0, 0)});
                    g_manager->fetchPtr<View>("view")->bindViewGeom(context.commandBuffer).drawPrimitives(context.commandBuffer);

                    rg.getBlackBoard().getImage(name).transitionLayout(context.commandBuffer, VulkanLayout::DEPTH_SAMPLER);
                    g_manager->getView()->setLightDirty(true);
                });
        }
    }
}
void ShadowMapPass::init() {
    PassBase::init();
    mPipelineLayout = std::make_unique<PipelineLayout>(g_context->getDevice().getResourceCache().requestPipelineLayout({"shadows/shadowMap.vert", "shadows/shadowMap.frag"}));
}
