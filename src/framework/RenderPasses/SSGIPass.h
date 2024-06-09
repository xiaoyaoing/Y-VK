#pragma once

#include "RenderPassBase.h"
#include "Core/PipelineLayout.h"
#include "Scene/SgImage.h"
struct SSRResource {
    std::unique_ptr<SgImage> depth_hierrachy{nullptr};
    std::unique_ptr<SgImage> blue_noise{nullptr};
};

class SSGIPass : public PassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;
    void updateGui() override;
    void update() override;
    ~SSGIPass() override = default;

protected:
    struct SSRPushConstant {
        uint32_t   use_inverse_depth{1};
        uint32_t   use_hiz{1};
        glm::uvec2 screen_size;
        float      depth_buffer_thickness{0.015};
        uint32_t   hiz_mip_count;
        uint32_t   show_original;
        float      pdding3;
    };

    std::unique_ptr<SSRResource>    mResource;
    std::unique_ptr<PipelineLayout> mPipelineLayout;
    SSRPushConstant                 mPushConstant{};
};
