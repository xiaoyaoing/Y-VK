#pragma once
#include "Core/PipelineLayout.h"
#include "Core/Texture.h"
#include "Scene/SgImage.h"

class RenderGraph;
class IBL {
public:
    void generatePrefilteredCubeMap(RenderGraph& rg);
    void generatePrefilertedEnvMap(RenderGraph& rg);
    void generateBRDFLUT(RenderGraph& rg);
    void importTexturesToRenderGraph(RenderGraph& rg);
    void generate(RenderGraph& rg);
    IBL(Device& device, const Texture*);
    ~IBL();

protected:
    bool                            generated{false};
    std::unique_ptr<PipelineLayout> cubeMapLayout;
    std::unique_ptr<PipelineLayout> envMapLayout;
    std::unique_ptr<PipelineLayout> brdfLUTLayout;
    std::unique_ptr<SgImage>                     irradianceCube{nullptr}, prefilterCube{nullptr}, brdfLUT{nullptr};
    const Texture*                  environmentCube{nullptr};
    Device&                         device;
};

extern IBL* g_ibl;