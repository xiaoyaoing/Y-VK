#pragma once

#include "RenderPassBase.h"

//Generate Hiz Buffer for depth Texture
//Like mipmap,but texel use  the maximum depth value in the 2x2 block
class HizPass {
public:
    static void render(RenderGraph& rg);//override;
};
