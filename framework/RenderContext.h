//
// Created by 打工人 on 2023/3/19.
//
#pragma  once

#include "Vulkan.h"
#include "Images/ImageView.h"
#include "RenderFrame.h"
#include <RenderTarget.h>

#include "Pipeline.h"
#include "RenderGraph/RenderGraph.h"
#include "ResourceBindingState.h"
#include "Scene/gltfloader.h"
#include "Utils/BufferPool.h"

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

    std::unique_ptr<CommandBuffer> commandBuffer{nullptr};
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

    void bindBuffer(uint32_t setId, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize range, uint32_t binding,
                    uint32_t array_element);

    void bindImage(uint32_t setId, const ImageView& view, const Sampler& sampler, uint32_t binding,
                   uint32_t array_element);

    void bindInput(uint32_t setId, const ImageView& view, uint32_t binding, uint32_t array_element);

    void bindMaterial(const gltfLoading::Material& material);

    void bindPrimitive(const gltfLoading::Primitive & primitive);

    void bindPipelineLayout(PipelineLayout& layout);

    void pushConstants(std::vector<uint8_t> pushConstants);

    void clearPassResources();

    void flushPipelineState(CommandBuffer& commandBuffer);

    
    void flushPushConstantStage(CommandBuffer & commandBuffer);


    const std::unordered_map<uint32_t, ResourceSet>& getResourceSets() const;



    RenderContext(Device& device, VkSurfaceKHR surface, Window& window);

    static RenderContext* g_context;

    VkFormat getSwapChainFormat() const;

    VkExtent2D getSwapChainExtent() const;

    uint32_t getSwapChainImageCount() const;

    void prepare();


    CommandBuffer& beginFrame();

    void waitFrame();

    uint32_t getActiveFrameIndex() const;

    void submit(CommandBuffer& buffer, VkFence fence = VK_NULL_HANDLE);



    FrameBuffer& getFrameBuffer();

    VkSemaphore submit(const Queue& queue, const std::vector<CommandBuffer*>& commandBuffers, VkSemaphore waitSem,
                       VkPipelineStageFlags waitPiplineStage);

    void setActiveFrameIdx(int idx);

    bool isPrepared() const;


    RenderGraph& getRenderGraph() const;

    PipelineState& getPipelineState();

    SgImage& getCurHwtexture();


    void flushDescriptorState(CommandBuffer& commandBuffer, VkPipelineBindPoint pipeline_bind_point);

    
    void flushAndDrawIndexed(CommandBuffer& commandBuffer, uint32_t indexCount, uint32_t instanceCount,
                             uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

    void flushAndDraw(CommandBuffer& commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                      uint32_t firstInstance);


    void beginRenderPass(CommandBuffer& commandBuffer, RenderTarget& renderTarget,
                         const std::vector<SubpassInfo>& subpassInfos);

    void nextSubpass(CommandBuffer& commandBuffer);

    void flush(CommandBuffer& commandBuffer);

    void endRenderPass(CommandBuffer& commandBuffer);

    BufferAllocation allocateBuffer(VkDeviceSize allocateSize, VkBufferUsageFlags usage);

    CommandBuffer& getCommandBuffer();

    const Camera* camera;

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
    } semaphores;

    PipelineState pipelineState;
    
    std::vector<std::unique_ptr<FrameResource>> frameResources{};
    
    std::vector<SgImage> hwTextures;
    
    std::unordered_map<uint32_t, ResourceSet> resourceSets;
    std::vector<uint8_t> storePushConstants;
    uint32_t maxPushConstantSize;
};
