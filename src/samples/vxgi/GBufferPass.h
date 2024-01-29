#pragma once
#include "VxgiCommon.h"

class GBufferPass : public VxgiPassBase {
public:
    void render(RenderGraph& rg) override;
    void init() override;

private:
    std::unique_ptr<SgImage>        mAlbedo{nullptr};
    std::unique_ptr<SgImage>        mNormal{nullptr};
    std::unique_ptr<PipelineLayout> mPipelineLayout{nullptr};
    Device&                         device;
};