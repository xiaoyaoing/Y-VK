#include <exception>
#include <iostream>
#include <fstream>
#include <memory>
#include <set>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <optional>

#include "Engine/Vulkan.h"
#include "Engine/VertexData.h"
#include "Engine/Buffer.h"
#include "Engine/CommandBuffer.h"
#include "Engine/Pipeline.h"
#include "Engine/FrameBuffer.h"
#include "Engine/RenderPass.h"
#include "Engine/Device.h"
#include <Engine/Images/Image.h>
#include "Engine/Queue.h"
#include "Engine/Window.h"
#include <Engine/Command/CommandPool.h>
#include "Engine/Instance.h"
#include "Engine/SwapChain.h"


#include "ext/stb_image/stb_image.h"
#include "Engine/Images/ImageView.h"
#include "Engine/Images/Sampler.h"
#include "Engine/Mesh.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <Engine/Descriptor/DescriptorPool.h>
#include <Engine/Descriptor/DescriptorLayout.h>
#include <Engine/Descriptor/DescriptorSet.h>

#include <chrono>
#include <thread>


/**
 * @file
 * @author JunPing Yuan
 * @brief
 * @version 0.1
 * @date 2023/2/18
 *
 * @copyright Copyright (c) 2022
 *
 */

///创建debug信息

static const int MAX_FRAMES_IN_FLIGHT = 2;
static std::string parentPath = "/Users/yjp/nju/大三下/graphics/vulkan学习/vulkan/";

static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const
VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger, const
VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, debugMessenger, pAllocator);
    }
}


struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class VKApp {
public:
    void run() {
        initWindow();
        initVk();
        mainloop();
        cleanup();
    }
    ~VKApp(){

    }
private:
    void
    populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType =
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    std::vector<const char *> getRequiredExtensions() {
        uint32_t glfwExtensionsCount = 0;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        return extensions;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, nullptr);
        if (presentCount != 0) {
            details.presentModes.resize(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentCount, details.presentModes.data());
        }

        return details;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: validationLayers) {
            bool layerFound = false;

            for (const auto &layerProp: availableLayers)
                if (strcmp(layerName, layerProp.layerName) == 0) {
                    layerFound = true;
                    break;
                }

            if (!layerFound) return false;
        }

        return true;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(_window->getHandle(), &width, &height);
            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)};
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height,
                                             capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {

        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }



    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

        for (int i =  0 ;i<queueFamilyProperties.size();i++) {
            const auto & queueFamily  = queueFamilyProperties[i];
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;
            if (indices.isComplete())
                break;
        }
        return indices;
    }
    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_device->getPhysicalDevice());

        _swapChain = std::make_shared<SwapChain>(_device, surface, _window);
        auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        auto presentMode = choosePresentMode(swapChainSupport.presentModes);
        auto extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(_device->getHandle(), _swapChain->getHandle(), &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(_device->getHandle(), _swapChain->getHandle(), &imageCount, swapChainImages.data());

    }

    void createSwapChainImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo;
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _swapChain->getImageFormat();
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            createInfo.pNext = nullptr;
            if (vkCreateImageView(_device->getHandle(), &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }

        }
    }

    void createImageViews() {
        imageViews.reserve(1);
        for (size_t i = 0; i < 1; i++)
            imageViews.push_back(std::make_shared<ImageView>(_device, _images[0], VK_IMAGE_ASPECT_COLOR_BIT, 1));
        imageSampler = std::make_shared<Sampler>(_device, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_FILTER_LINEAR, 0);
    }

    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _swapChain->getImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,depthAttachment};


        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT & VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachments.size();
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        VkRenderPass _pass;
        auto result = vkCreateRenderPass(_device->getHandle(), &renderPassInfo, nullptr, &_pass);
        if ( result != VK_SUCCESS) {
            RUN_TIME_ERROR("Failed to create render pass")
        }
        renderPass = std::make_shared<RenderPass>(_pass);

    }

    void createGraphicsPipline() {
        auto vertShaderCode = readFile(parentPath + "shaders/vert.spv");
        auto fragShaderCode = readFile(parentPath + "shaders/frag.spv");

        auto vertShaderModule = createShaderModule(vertShaderCode);
        auto fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) _swapChain->getExtent().width;
        viewport.height = (float) _swapChain->getExtent().height;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = _swapChain->getExtent();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        viewportState.pNext = nullptr;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;
        rasterizer.pNext = nullptr;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType =
                VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;


        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                              VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                              VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType =
                VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 0;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        VkDescriptorSetLayout pSetLayouts[] = {_descriptorLayout->getHandle()};
        pipelineLayoutCreateInfo.pSetLayouts = pSetLayouts;
        if (vkCreatePipelineLayout(_device->getHandle(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            RUN_TIME_ERROR("Failed to create pipeline layout");

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0;
        depthStencil.maxDepthBounds = 1.0;


        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        // pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = nullptr;
        pipelineInfo.layout = pipelineLayout;

        pipelineInfo.renderPass = renderPass->getHandle();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline pipeline;
        if (vkCreateGraphicsPipelines(_device->getHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        graphicsPipeline = std::make_shared<Pipeline>(pipeline);


        vkDestroyShaderModule(_device->getHandle(), fragShaderModule, nullptr);
        vkDestroyShaderModule(_device->getHandle(), vertShaderModule, nullptr);

    }

    VkShaderModule createShaderModule(const std::vector<char> &code) {
        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());
        createInfo.pNext = nullptr;
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_device->getHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            RUN_TIME_ERROR("Failed to create shader module")
        }
        return shaderModule;

    }

    void createFrameBuffers() {
        swapChainFrameBuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            std::vector<VkImageView> attachments = {swapChainImageViews[i],_depthImageView->getHandle()};
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass->getHandle();
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = _swapChain->getExtent().width;
            framebufferInfo.height = _swapChain->getExtent().height;
            framebufferInfo.layers = 1;
            VkFramebuffer _buffer;
            if (vkCreateFramebuffer(_device->getHandle(), &framebufferInfo, nullptr, &_buffer) != VK_SUCCESS)
                RUN_TIME_ERROR("Failed to create frameBuffer");

            swapChainFrameBuffers[i] = std::make_shared<Framebuffer>(_buffer);
        }
    }

    void createCommandPool() {

        commandPool = std::make_shared<CommandPool>(_device, graphicsQueue,
                                                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    }

    void createCommandBuffer() {
        commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = commandPool->getHandle();
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        std::vector<VkCommandBuffer> vkCommandBuffers(MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateCommandBuffers(_device->getHandle(), &allocateInfo, vkCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        for (const auto &vkCommandBuffer: vkCommandBuffers)
            commandBuffers.emplace_back(std::make_shared<CommandBuffer>(vkCommandBuffer));
    }

    void createSyncObjects() {
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            if (vkCreateSemaphore(_device->getHandle(), &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateSemaphore(_device->getHandle(), &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
                vkCreateFence(_device->getHandle(), &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                RUN_TIME_ERROR("Failed to create semaphores")

            }

    }


    void createAllocator() {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = _device->getPhysicalDevice();
        allocatorInfo.device = _device->getHandle();
        allocatorInfo.instance = _instance->getHandle();
        ASSERT(vmaCreateAllocator(&allocatorInfo, &allocator) == VK_SUCCESS, "create allocator");
    }

    void createDescriptorSetLayout() {
        _descriptorLayout = std::make_shared<DescriptorLayout>(_device);
        _descriptorLayout->addBinding(VK_SHADER_STAGE_VERTEX_BIT, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
        _descriptorLayout->addBinding(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
        _descriptorLayout->createLayout(0);
    }

    void createDescriptorSet() {
        _descriptorSet.reserve(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            _descriptorSet.push_back(std::make_shared<DescriptorSet>(_device, _descriptorPool, _descriptorLayout, 1));
            _descriptorSet[i]->updateBuffer(_uniformBuffers[curFrameCount], 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            VkDescriptorImageInfo imageInfo;
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = imageViews[0]->getHandle();
            imageInfo.sampler = imageSampler->getHandle();
            _descriptorSet[i]->updateImage({imageInfo}, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        }
    }

    void updateUniformBuffer() {

        static auto startTime =
                std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float,
                std::chrono::seconds::period>(currentTime - startTime).count();
        //todo modify
        time = 0;
        UniformBufferObject ubo{};

        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.f),
                                glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), _swapChain->getExtent().width / (float) _swapChain->getExtent().height, 0.1f,
                                    10.0f);
        ubo.proj[1][1] *= -1;
        _uniformBuffers[curFrameCount][0]->uploadData(&ubo, sizeof(UniformBufferObject));
    }

    void createDescriptorPool() {
        std::vector<VkDescriptorPoolSize> poolSizes = {
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         MAX_FRAMES_IN_FLIGHT},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT}
        };
        _descriptorPool = std::make_shared<DescriptorPool>(_device, poolSizes, MAX_FRAMES_IN_FLIGHT, 0);
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        _uniformBuffers.assign(MAX_FRAMES_IN_FLIGHT, std::vector<ptr<Buffer>>());
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            for (size_t j = 0; j < 1; j++)
                _uniformBuffers[i].push_back(
                        std::make_shared<Buffer>(allocator, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                 VMA_MEMORY_USAGE_CPU_TO_GPU));
        }
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_device->getPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        RUN_TIME_ERROR("failed to find supported format!")
    }

    void createImages() {
        // commandPool->

        _images.resize(1);
        int width, height, channels;
        auto imagePath = parentPath + "resources/Window.png";
        auto imageData = stbi_load(imagePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
        auto buffer = std::make_shared<Buffer>(allocator, 4 * width * height, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                               VMA_MEMORY_USAGE_CPU_TO_GPU);
        buffer->uploadData(imageData, 4 * width * height);
        auto imageInfo = Image::getDefaultImageInfo();
        imageInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        _images[0] = std::make_shared<Image>(allocator, VMA_MEMORY_USAGE_GPU_ONLY, imageInfo);
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
        auto commandBuffer = std::make_shared<CommandBuffer>(commandPool->allocateCommandBuffer());
        commandBuffer->beginRecord(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        commandBuffer->copyBufferToImage(buffer, _images[0], {region});
        commandBuffer->endRecord();
        graphicsQueue->submit({commandBuffer}, nullptr);
        vkQueueWaitIdle(graphicsQueue->getHandle());

    }

    void createDepthResources() {
        auto depthImageInfo = Image::getDefaultImageInfo();
        depthImageInfo.extent = VkExtent3D{_swapChain->getExtent().width, _swapChain->getExtent().height, 1};
        depthImageInfo.format = VK_FORMAT_D32_SFLOAT;
        depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        _depthImage = std::make_shared<Image>(allocator, VMA_MEMORY_USAGE_GPU_ONLY, depthImageInfo);
        _depthImageView = std::make_shared<ImageView>(_device, _depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }
    void addDeviceExtension(const char *extension, bool optional){
        deviceExtensions[extension] = optional;
    }
    void addInstanceExtension(const char *extension, bool optional){
        instanceExtensions[extension] = optional;
    }

    void createMeshes() {
        meshes.reserve(1);
        meshes.push_back(std::make_shared<Mesh>("/Users/yjp/nju/大三下/graphics/vulkan学习/vulkan/resources/viking_room.obj"));
        meshes[0]->createBuffer(allocator);
    }

    void initVk() {
        //todo queue family index
        //  createInstance();
        _instance = std::make_shared<Instance>("vulkanApp",requiredExtensions,validationLayers);

        uint32_t physicalDeviceCount;
        VkPhysicalDevice  * physicalDevices;
        vkEnumeratePhysicalDevices(_instance->getHandle(),&physicalDeviceCount,physicalDevices);
        auto physicalDevice = physicalDevices[0];

        surface = _window->createSurface(_instance->getHandle());
        _device = std::make_shared<Device>(physicalDevice,surface);
        graphicsQueue = _device->getQueueByFlag(VK_QUEUE_GRAPHICS_BIT,0);
        presentQueue = _device->getPresentQueue(0);

     //   createAllocator();

        createDescriptorSetLayout();

      //  createSwapChain();
      //  createSwapChainImageViews();
       // createRenderPass();
        createGraphicsPipline();

      //  createDepthResources();
      //  createFrameBuffers();
        createCommandPool();

        //   createVertexBuffers();
        //    createIndiceBuffers();
        createUniformBuffers();
        createMeshes();

        createImages();
        createImageViews();

        createDescriptorPool();
        createDescriptorSet();

        createCommandBuffer();
        createSyncObjects();
        glfwSetWindowUserPointer(_window->getHandle(), this);
    }

    void drawFrame()
        vkWaitForFences(_device->getHandle(), 1, &inFlightFences[curFrameCount], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        auto result = vkAcquireNextImageKHR(_device->getHandle(), _swapChain->getHandle(), UINT64_MAX, imageAvailableSemaphores[curFrameCount],
                                            VK_NULL_HANDLE,
                                            &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            reCreateSwapChain();
            return;
        }

        updateUniformBuffer();

        vkResetFences(_device->getHandle(), 1, &inFlightFences[curFrameCount]);


        auto commandBuffer = commandBuffers[curFrameCount];
        commandBuffer->beginRecord(1);
        std::vector<VkClearValue> clearValues(2);
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        commandBuffer->beginRenderPass(renderPass->getHandle(), swapChainFrameBuffers[curFrameCount]->getHandle(),
                                       clearValues, _swapChain->getExtent());

        for(const auto & mesh : meshes)
            mesh->bindOnly(commandBuffer->getHandle());
        commandBuffer->bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                          {_descriptorSet[curFrameCount]}, {});
        commandBuffer->bindPipeline(graphicsPipeline->getHandle());
        for(const auto & mesh : meshes)
            mesh->drawOnly(commandBuffer->getHandle());
        commandBuffer->endRecord();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[curFrameCount]};
        VkPipelineStageFlags waitStages[] =
                {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        VkCommandBuffer submitCommandBuffers[] = {commandBuffers[curFrameCount]->getHandle()};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = submitCommandBuffers;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[curFrameCount]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        result = vkQueueSubmit(graphicsQueue->getHandle(), 1, &submitInfo, inFlightFences[curFrameCount]);
        if (result != VK_SUCCESS) {
            RUN_TIME_ERROR("failed to submit queue buffer");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {_swapChain->getHandle()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        //vkDeviceWaitIdle(device);
        //std::this_thread::sleep_for(std::chrono::milliseconds (10));

        result = vkQueuePresentKHR(presentQueue->getHandle(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
            frameBufferResized = false;
            reCreateSwapChain();
            return;
        }

        curFrameCount = (curFrameCount + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void mainloop() {
        while (!glfwWindowShouldClose(_window->getHandle())) {
            glfwPollEvents();
            drawFrame();
        }
    }

    void cleanupSwapChain() {
        for (int i = 0; i < swapChainFrameBuffers.size(); i++) {
            swapChainFrameBuffers[i]->cleanup();
            // vkDestroyFramebuffer(device, swapChainFrameBuffers[i], nullptr);
        }
        graphicsPipeline->cleanup();
        //vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_device->getHandle(), pipelineLayout, nullptr);
        //  vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroySwapchainKHR(_device->getHandle(), _swapChain->getHandle(), nullptr);

        for (int i = 0; i < swapChainImageViews.size(); i++) {
            vkDestroyImageView(_device->getHandle(), swapChainImageViews[i], nullptr);

        }
    }

    void reCreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(_window->getHandle(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(_window->getHandle(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(_device->getHandle());
        cleanupSwapChain();

        createSwapChain();
        createSwapChainImageViews();
        createRenderPass();
        createGraphicsPipline();
        createFrameBuffers();


        vmaDestroyAllocator(allocator);
    }

    void cleanup() {
//        cleanupSwapChain();
//
//        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//            vkDestroyFence(_device->getHandle(), inFlightFences[i], nullptr);
//            vkDestroySemaphore(_device->getHandle(), imageAvailableSemaphores[i], nullptr);
//            vkDestroySemaphore(_device->getHandle(), renderFinishedSemaphores[i], nullptr);
//        }
//        vkDestroyDevice(_device->getHandle(), nullptr);
//
//
//        glfwDestroyWindow(_window->getHandle());
//        glfwTerminate();
    }

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        _window = std::make_shared<Window>(glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr));
        glfwSetFramebufferSizeCallback(_window->getHandle(), resizeCallBack);
    }

    static void resizeCallBack(GLFWwindow *window, int width, int height) {
        VKApp *app = reinterpret_cast<VKApp *>(glfwGetWindowUserPointer(window));
        app->frameBufferResized = true;
    }

    static VKAPI_ATTR VkBool32  VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData
    ) {
        return VK_FALSE;
    }
    ptr<Window>  _window;

    ptr<Device> _device;
    ptr<Instance> _instance;
    ptr<Queue> graphicsQueue, presentQueue;

    ptr<SwapChain> _swapChain;

    VkSurfaceKHR surface;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<ptr<Framebuffer>> swapChainFrameBuffers;

    ptr<RenderPass> renderPass;
    VkPipelineLayout pipelineLayout;
    ptr<Pipeline> graphicsPipeline;

    ptr<CommandPool> commandPool;

    std::vector<ptr<CommandBuffer>> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    int curFrameCount = 0;
    bool frameBufferResized = false;

    std::vector<std::vector<ptr<Buffer>>> _uniformBuffers;
    std::vector<ptr<Image>> _images;
    std::vector<ptr<ImageView>> imageViews;
    ptr<Image> _depthImage;
    ptr<ImageView> _depthImageView;
    ptr<Sampler> imageSampler;

    std::vector<ptr<Mesh>> meshes;

    VmaAllocator allocator;

    std::vector<ptr<DescriptorSet>> _descriptorSet;
    ptr<DescriptorPool> _descriptorPool;
    ptr<DescriptorLayout> _descriptorLayout;

    std::unordered_map<const char *, bool> deviceExtensions;
    /** @brief Set of instance extensions to be enabled for this example and whether they are optional (must be set in the derived constructor) */
    std::unordered_map<const char *, bool> instanceExtensions;

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
#ifdef  NOEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};

int main() {
    VKApp app;
    try {
        app.run();
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
};