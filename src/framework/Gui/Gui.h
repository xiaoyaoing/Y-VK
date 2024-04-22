#pragma once

#include "Core/Vulkan.h"
#include "Core/Buffer.h"
#include <glm/vec2.hpp>

#include "InputEvent.h"
#include "Core/BufferPool.h"
#include "Core/PipelineState.h"
#include "Core/Texture.h"
#include "Core/Descriptor/DescriptorSet.h"
//#include "App/Application.h"

class RenderContext;

class Image;

class ImageView;

class Application;

class CommandBuffer;

class RenderGraph;

class Gui {
public:
    Gui(Device& device);

    bool inputEvent(const InputEvent& event);
    void prepareResoucrces(Application* app);
    bool update();
    void addGuiPass(RenderGraph& graph);
    void setColorsDark();
    bool checkBox(const char* caption, bool* value);
    void text(const char* formatstr, ...);
    void newFrame();

    //是否有新的数据 需要更新
    bool  updated{false};
    float scale{1.f};

protected:
    VkPipeline pipeline{};

    //Gui buffer may be updated every frame so we use a frame buffer pool to handle this
    BufferAllocation mvertexBuffer{}, mIndexBuffer{};

    uint32 vertexCount, indexCount;

    std::vector<VkPipelineShaderStageCreateInfo> shaders;

    //    VkDescriptorPool descriptorPool;
    //    VkDescriptorSetLayout descriptorSetLayout;
    //    VkDescriptorSet descriptorSet;
    //  VkPipelineLayout pipelineLayout;

    std::unique_ptr<DescriptorSet>    descriptorSet{nullptr};
    std::unique_ptr<DescriptorPool>   descriptorPool{nullptr};
    std::unique_ptr<DescriptorLayout> descriptorLayout{nullptr};

    float contentScaleFactor{1.f};

    VkSampleCountFlagBits rasterSamples = VK_SAMPLE_COUNT_1_BIT;

    Device& device;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
        int       flipViewPort;
        glm::vec3 padding;
    } pushConstBlock;

    PipelineLayout* pipelineLayout;

    VertexInputState vertexInputState;

    ColorBlendState colorBlendState;

    std::unique_ptr<Texture> fontTexture;
    uint32                   subPass{0};
    double                   mTime{0};
};