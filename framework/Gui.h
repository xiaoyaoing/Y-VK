#pragma once

#pragma once

#include <imgui.h>
#include <Vulkan.h>
#include <Buffer.h>
#include <glm/vec2.hpp>
#include "API_VK.h"
#include "Descriptor/DescriptorSet.h"
//#include "App/Application.h"

class RenderContext;
class Image;

class ImageView;

class Application;

class CommandBuffer;

class RenderGraph;

class Gui {
public:
    Gui(Device &device);

    void prepare(const VkPipelineCache pipelineCache, const VkRenderPass renderPass);

    void prepareResoucrces(Application *app);

    bool update();

    void draw(VkCommandBuffer commandBuffer);

    void addGuiPass(RenderGraph & graph,RenderContext & renderContext);

    bool checkBox(const char *caption, bool *value);

    void text(const char *formatstr, ...);

//是否有新的数据 需要更新
    bool updated{false};
    float scale{1.f};
protected:

    VkPipeline pipeline{};

    std::unique_ptr<Buffer> vertexBuffer{nullptr}, indexBuffer{nullptr};

    uint32 vertexCount, indexCount;

    std::vector<VkPipelineShaderStageCreateInfo> shaders;

//    VkDescriptorPool descriptorPool;
//    VkDescriptorSetLayout descriptorSetLayout;
//    VkDescriptorSet descriptorSet;
  //  VkPipelineLayout pipelineLayout;

    std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    std::unique_ptr<DescriptorPool> descriptorPool{nullptr};
    std::unique_ptr<DescriptorLayout> descriptorLayout{nullptr};


    VkSampleCountFlagBits rasterSamples = VK_SAMPLE_COUNT_1_BIT;

    Device &device;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;


    PipelineLayout * pipelineLayout;
    

    Texture fontTexture;
    uint32 subPass{0};

};
