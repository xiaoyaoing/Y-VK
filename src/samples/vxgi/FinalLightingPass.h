#pragma once
#include "VxgiCommon.h"
#include "Core/Images/Sampler.h"
#include "RenderPasses/RenderPassBase.h"

class FinalLightingPass : public PassBase {
public:
    void init() override;
    void render(RenderGraph& rg) override;
    void pushFinalLightingParam();
    void updateGui() override;

protected:
    struct FinalLightingParam {
        glm::vec3 volume_center;                // 12
        uint32_t  uRenderingMode{0};            // 16
        float     voxel_size;                   // 20
        float     clip_map_resoultion;          // 24
        float     uTraceStartOffset{0.05};      // 28
        float     uIndirectDiffuseIntensity{1}; // 32
        float     uAmbientOcclusionFactor{1};   // 36
        float     uMinTraceStepFactor{1};       // 40
        float     uIndirectSpecularIntensity{1};// 44
        float     uOcclusionDecay{1};           // 48
        int       uEnable32Cones{0};
        int       uDirectLighting{1};
        int       uIndirectLighting{1};
    };
    std::unique_ptr<PipelineLayout> mFinalLightingPipelineLayout{nullptr};
    std::unique_ptr<Sampler>        mRadianceMapSampler{nullptr};
    std::unique_ptr<Buffer>         mVoxelParamBuffer{nullptr};
    uint32_t                        frameIndex{0};
    FinalLightingParam              mVoxelParam{};
};
