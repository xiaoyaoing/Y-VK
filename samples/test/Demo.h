#pragma once

#include <Engine/App/Application.h>

class Demo : public Application{
public:
private:
    void initVk() override;

    void drawFrame() override;

protected:
    void createGraphicsPipeline();
    void createMeshes();
    void createImages();
    void createUniformBuffers();
    void createDescriptorSet();
    void updateUnifomBuffers();
protected:
    DescriptorPool _descriptorPool;
    DescriptorLayout _descriptorLayout;
    DescriptorSet _descriptorSet;
    ptr<Pipeline> _pipeline;
};
