#include "LightInjectionPass.h"

#include "Scene/Scene.h"
void LightInjectionPass::render(RenderGraph& rg) {
    auto& blackboard = rg.getBlackBoard();
    rg.addPass(
        "LightInjectionPass", [&](auto& builder, auto& settings) {
     //   auto opacity = blackboard.getHandle("opacity");
        auto voxelRadiance = rg.getBlackBoard().getHandle("voxelRadiance");
        auto normal = rg.getBlackBoard().getHandle("normal");
        auto depth = rg.getBlackBoard().getHandle("depth");
        auto albedo = rg.getBlackBoard().getHandle("albedo");
        RenderGraphPassDescriptor desc{};
        desc.textures = {voxelRadiance,normal,depth,albedo};
        desc.addSubpass({.inputAttachments =  {albedo, normal, depth},.outputAttachments = {voxelRadiance}});
        ("Light Injection Pass",desc); }, [&](auto& renderpassContext) {
            mScene->IteratePrimitives([](const auto& primitive) {
                    
                                                                                                     }); });
}
void LightInjectionPass::init() {
}