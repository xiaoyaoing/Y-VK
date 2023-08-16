//
// Created by pc on 2023/8/16.
//
#pragma once

#include "Application.h"

class ApiAPP : public Application {

protected:
    void buildCmdBuffers();

//    void createRenderPass();

    void drawFrame() override;

    std::vector<VkCommandBuffer> draw_cmd_buffers;

    VkRenderPass render_pass;
};


