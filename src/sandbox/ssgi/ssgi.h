//
// Created by pc on 2024/1/24.
//

#ifndef VULKANDEMO_TRIANGLE_H
#define VULKANDEMO_TRIANGLE_H

#include <App/Application.h>

class PBRLab : public Application {
public:
    void drawFrame(RenderGraph& rg) override;
    PBRLab();
    void prepare() override;
    void onUpdateGUI() override;

protected:
    std::unique_ptr<PipelineLayout>                        layout{nullptr};
    VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterizationProperties{};
    VkPipelineRasterizationConservativeStateCreateInfoEXT  conservativeRasterizationStateCreateInfo{};
    bool                                                   rasterizationConservative = true;
};

#endif//VULKANDEMO_TRIANGLE_H