//
// Created by pc on 2023/8/17.
//

#include <random>
#include "Shader.h"
#include "FIleUtils.h"
#include "App/Application.h"
#include "Common/ResourceCache.h"
#include "RenderGraph/RenderGraph.h"
#include "RenderGraph/RenderGraphId.h"


class ShaderReflectionTest : public Application {
public:
    void drawFrame() override {
        auto &graph = renderContext->getRenderGraph();
        struct GBufferPassData {
            RenderGraphId<RenderGraphTexture> shadows;

            RenderGraphId<RenderGraphTexture> depth;

            RenderGraphId<RenderGraphTexture> color;

            RenderGraphId<RenderGraphTexture> output;
        };

        graph.addPass<GBufferPassData>("gbuffer", [&](RenderGraph::Builder &builder, GBufferPassData &data) {
                                           builder.declare("Color Pass Target", {
                                                                   .color = {data.color, data.output},
                                                                   .depth = {data.depth}
                                                           }
                                           );
                                       },
                                       [&](GBufferPassData &data, RenderPassContext &context) {
                                           auto &commandBuffer = context.commandBuffer;
                                           commandBuffer.bindPipeline(context.pipeline);

                                           for (auto &mesh: scene->meshes) {
                                               mesh->bindAndDraw(commandBuffer);
                                           }

                                           //first  set shader
                                           //set descriptor 
                                           //  auto& pipeline = device->getResourceCache().requ;
                                       });
    }

    void initVk() override {
        getRequiredInstanceExtensions();
        _instance = std::make_unique<Instance>(std::string("vulkanApp"), instanceExtensions, validationLayers);
        surface = window->createSurface(*_instance);

        // create device
        uint32_t physical_device_count{0};
        VK_CHECK_RESULT(vkEnumeratePhysicalDevices(_instance->getHandle(), &physical_device_count, nullptr));

        if (physical_device_count < 1) {
            throw std::runtime_error("Couldn't find a physical device that supports Vulkan.");
        }

        std::vector<VkPhysicalDevice> physical_devices;
        physical_devices.resize(physical_device_count);

        LOGI("Found {} physical device", physical_device_count);

        VK_CHECK_RESULT(
                vkEnumeratePhysicalDevices(_instance->getHandle(), &physical_device_count, physical_devices.data()));

        addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physical_devices[0], &deviceProperties);
        LOGI("Device Name: {}", deviceProperties.deviceName)

        device = std::make_unique<Device>(physical_devices[0], surface, deviceExtensions);


        std::vector<Shader> shaders;
        shaders.emplace_back(*device, FileUtils::getShaderPath() + "instancing.vert");

        shaders.emplace_back(*device, FileUtils::getShaderPath() + "instancing.frag");

        RenderGraph graph(*device);


        struct RenderPassDesc {
            std::vector<SubpassInfo> &subpasses;
            std::vector<Attachment> colorAttachments;
            Attachment depthAttachment;
        };


        auto descriptor = device->getResourceCache().requestDescriptorLayout(shaders);
        exit(0);
    }
};


int main() {
    ShaderReflectionTest shader_reflection_test;
    shader_reflection_test.prepare();
}
