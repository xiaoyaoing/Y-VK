//
// Created by 打工人 on 2023/3/19.
//
#pragma  once

#include "Core/Vulkan.h"
#include "Core/Images/ImageView.h"

#include "RenderTarget.h"

#include "Pipeline.h"
#include "RenderGraph/RenderGraph.h"
#include "ResourceBindingState.h"
#include "Scene/SceneLoader/gltfloader.h"
#include "Core/BufferPool.h"

class Device;

class SwapChain;

class Window;

class FrameBuffer;

class Sampler;

class Scene;


struct FrameResource
{
    static constexpr uint32_t BUFFER_POOL_BLOCK_SIZE = 256;

    const std::unordered_map<VkBufferUsageFlags, uint32_t> supported_usage_map = {
        {VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 1},
        {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 2},
        // x2 the size of BUFFER_POOL_BLOCK_SIZE since SSBOs are normally much larger than other types of buffers
        {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 1},
        {VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 1}
    };

    FrameResource(Device&);

    ~FrameResource() = default;

    void reset();

    std::unique_ptr<CommandBuffer> graphicComputeBuffer{nullptr};
    std::unique_ptr<CommandBuffer> computeComputeBuffer{nullptr};

    
    std::unordered_map<VkBufferUsageFlags, std::unique_ptr<BufferPool>> bufferPools{};
    // CommandBuffer commandBuffer;
};

struct alignas(16) GlobalUniform
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};


class RenderContext
{
public:
    // Resource Functions Begin
    
    RenderContext(Device& device, VkSurfaceKHR surface, Window& window);

    static RenderContext* g_context;
    

    // RenderContext & bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding,
    //                 uint32_t array_element);
    
    RenderContext &  bindBuffer(uint32_t binding, const Buffer &buffer, VkDeviceSize offset = 0 , VkDeviceSize range = 0 ,uint32_t setId = 0,uint32_t array_element=0);
    

    RenderContext & bindAcceleration(uint32_t setId, const Accel& acceleration, uint32_t binding,
                          uint32_t array_element);

    RenderContext & bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding,
                   uint32_t array_element);

    RenderContext & bindInput(uint32_t setId, const ImageView& view, uint32_t binding, uint32_t array_element);

    RenderContext & bindMaterial(const Material& material);

    RenderContext & bindPrimitive(CommandBuffer & commandBuffer,const Primitive& primitive);


    template <typename T>
    RenderContext & bindLight(const std::vector<Light>& lights, uint32_t setId, uint32_t binding);

    RenderContext & bindPushConstants(std::vector<uint8_t>& pushConstants);

    void flushPipelineState(CommandBuffer& commandBuffer);
    
    void flushPushConstantStage(CommandBuffer& commandBuffer);

    void clearPassResources();
    
    const std::unordered_map<uint32_t, ResourceSet>& getResourceSets() const;

    VkPipelineBindPoint getPipelineBindPoint() const;

    VkFormat getSwapChainFormat() const;

    VkExtent2D getSwapChainExtent() const;

    uint32_t getSwapChainImageCount() const;

    void prepare();


    void beginFrame();

    void waitFrame();

    uint32_t getActiveFrameIndex() const;

    void submitAndPresent(CommandBuffer& buffer, VkFence fence = VK_NULL_HANDLE);
    void submit(CommandBuffer& commandBuffer, bool waiteFence = true);

    

    void setActiveFrameIdx(int idx);

    bool isPrepared() const;



    PipelineState& getPipelineState();

    SgImage& getCurHwtexture();


    void flushDescriptorState(CommandBuffer& commandBuffer, VkPipelineBindPoint pipeline_bind_point);


    void flushAndDrawIndexed(CommandBuffer& commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                             uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

    void flushAndDraw(CommandBuffer& commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance);

    void traceRay(CommandBuffer& commandBuffer,VkExtent3D dims);

    void dispath(CommandBuffer & commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

    void beginRenderPass(CommandBuffer& commandBuffer, RenderTarget& renderTarget,
                         const std::vector<SubpassInfo>& subpassInfos);

    void nextSubpass(CommandBuffer& commandBuffer);

    void flush(CommandBuffer& commandBuffer);

    void endRenderPass(CommandBuffer& commandBuffer,RenderTarget& renderTarget);

    BufferAllocation allocateBuffer(VkDeviceSize allocateSize, VkBufferUsageFlags usage);

    CommandBuffer& getGraphicBuffer();
    CommandBuffer& getComputeCommandBuffer();

    void handleSurfaceChanges();

    void recrateSwapChain(VkExtent2D extent);

    void copyBuffer(Buffer & src, Buffer & dst);

private:
    bool frameActive = false;
    VkSemaphore acquiredSem;
    bool prepared{false};
    uint32_t activeFrameIndex{0};

    Device& device;
    std::unique_ptr<SwapChain> swapchain;

    uint32_t swapChainCount{0};

    std::vector<std::unique_ptr<FrameBuffer>> frameBuffers;
    VkExtent2D surfaceExtent;

    struct
    {
        VkSemaphore presentFinishedSem;
        VkSemaphore renderFinishedSem;
        VkSemaphore computeFinishedSem;
        VkSemaphore graphicFinishedSem;
    } semaphores;

    PipelineState pipelineState;

    std::vector<std::unique_ptr<FrameResource>> frameResources{};

    std::vector<SgImage> hwTextures;

    std::unordered_map<uint32_t, ResourceSet> resourceSets;
    std::vector<uint8_t> storePushConstants;
    uint32_t maxPushConstantSize;
};

template <typename T>
RenderContext & RenderContext::bindLight(const std::vector<Light>& lights, uint32_t setId, uint32_t binding)
{
    const auto buffer = allocateBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);


    std::vector<LightUBO> directionalLights, pointLights, spotLights;

    for (const auto& light : lights)
    {
        LightUBO lightUBO{};
        switch (light.type)
        {
        case LIGHT_TYPE::Point:
            lightUBO = {
                .color = glm::vec4(light.lightProperties.color, light.lightProperties.intensity),
                .position = glm::vec4(light.lightProperties.position, 1),
                .direction = glm::vec4(0.0f),
                .info = glm::vec2(0.0f)
            };
            pointLights.push_back(lightUBO);
        default:
            break; //todo
        }
    }


    T lightUbos{};
    std::copy(directionalLights.begin(), directionalLights.end(), lightUbos.directionalLights);
    std::copy(pointLights.begin(), pointLights.end(), lightUbos.pointLights);
    std::copy(spotLights.begin(), spotLights.end(), lightUbos.spotLights);

    buffer.buffer->uploadData(&lightUbos, sizeof(T), buffer.offset);

    bindBuffer(setId, *buffer.buffer, buffer.offset, buffer.size, binding, 0);

    return *this;
}
