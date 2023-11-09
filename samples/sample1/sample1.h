//
// Created by pc on 2023/8/17.
//
#pragma once

#include "App/Application.h"

class Example : public Application {

public:
    void prepare() override;

    Example();

protected:
    struct UBOVS {
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                                      glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 view;
        glm::mat4 proj;
    } ubo_vs;

    struct {
        std::unique_ptr<Buffer> scene;
    } uniform_buffers;

    struct {
        Texture texture1;
    } textures;

    std::unique_ptr<DescriptorSet> descriptorSet{nullptr};
    std::unique_ptr<DescriptorPool> descriptorPool{nullptr};

    std::unique_ptr<Pipeline> graphicsPipeline{nullptr};


    void prepareUniformBuffers();

    void createDescriptorSetLayout();

    void createDescriptorSet();

    void createDescriptorPool();

    void updateUniformBuffers();

    void createGraphicsPipeline();

    void bindUniformBuffers(CommandBuffer &commandBuffer) override;

    void buildCommandBuffers() override;

    void updateScene() override {
        updateUniformBuffers();
    }

    void onUpdateGUI() override;

    void draw(CommandBuffer &commandBuffer) override;

};