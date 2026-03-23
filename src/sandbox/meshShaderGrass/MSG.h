//
// Created by pc on 2024/1/24.
//

#ifndef VULKANDEMO_TRIANGLE_H
#define VULKANDEMO_TRIANGLE_H

#include "RenderPasses/GrassPass.h"

#include <App/Application.h>

class MeshShaderGrass : public Application {
public:
    void drawFrame(RenderGraph& rg) override;
    MeshShaderGrass();
    void prepare() override;
    void onUpdateGUI() override;

protected:
    std::unique_ptr<GrassPass> grassPass{nullptr};
    
};

#endif//VULKANDEMO_TRIANGLE_H