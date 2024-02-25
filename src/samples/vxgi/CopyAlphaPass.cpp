#include "CopyAlphaPass.h"
void CopyAlphaPass::render(RenderGraph& rg) {
    rg.addComputePass("TrianglePass",
    [&](RenderGraph::Builder& builder, ComputePassSettings & settings){
    
    },
    [&](RenderPassContext& context) {
 
    });
}
void CopyAlphaPass::init() {
}
 