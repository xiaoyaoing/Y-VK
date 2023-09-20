//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"
#include "Scene/gltfloader.h"

class Example : public Application {

public:
    void prepare() override;

    void loadResources();


    Example();

protected:
    struct UBOVS {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec4 lightPos = glm::vec4(0.0f, -5.0f, 0.0f, 1.0f);
        float locSpeed = 0.0f;
        float globSpeed = 0.0f;
    } ubo_vs;

    struct {
        std::unique_ptr<Buffer> scene;
    } uniform_buffers;


    struct {
        std::unique_ptr<gltfLoading::Model> planets;
        std::unique_ptr<gltfLoading::Model> rockets;
    } models;

    struct {
        Texture planetTexture;
        Texture rocketTexture;
    } textures;

    struct {
        VkPipeline startFieldPipeline;
        VkPipeline planetPipeline;
        VkPipeline instancedRockPipeline;
    } pipelines;

    struct InstanceData {
        glm::vec3 pos;
        glm::vec3 rot;
        float scale;
        uint32_t texIndex;
    };

    struct {
        std::unique_ptr<Buffer> buffer{nullptr};
        std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    } instanceBuffer;


    struct {
        std::unique_ptr<DescriptorSet> rockDescriptor;
        std::unique_ptr<DescriptorSet> planetDescriptor;
    } descriptors;

    // std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    std::unique_ptr<DescriptorPool> descriptorPool{nullptr};

    void prepareUniformBuffers();

    void createDescriptorSetLayout();

    void createDescriptorSet();

    void createDescriptorPool();

    void updateUniformBuffers();

    void createGraphicsPipeline();

    void prepareInstanceData();

    void bindUniformBuffers(CommandBuffer &commandBuffer) override;

    void buildCommandBuffers() override;

    void updateScene() override {
        updateUniformBuffers();
    }

    void onUpdateGUI() override;

    void draw(CommandBuffer &commandBuffer) override;

};
