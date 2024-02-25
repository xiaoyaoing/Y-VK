#pragma once
#include "VxgiCommon.h"
class CopyAlphaPass : public  VxgiPassBase{
public:
    void render(RenderGraph& rg) override;
    void init() override;
};
