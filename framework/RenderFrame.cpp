//
// Created by 打工人 on 2023/3/28.
//

#include "RenderFrame.h"
#include <RenderTarget.h>

CommandBuffer &
RenderFrame::requestCommandBuffer(const Queue &queue, CommandBuffer::ResetMode reset_mode, VkCommandBufferLevel level) {
    //  return CommandBuffer();
//    return <#initializer#>;
    return *buffer;
}

RenderTarget &RenderFrame::getRenderTarget() {
    return *renderTarget;
}

RenderFrame::RenderFrame(Device &device, std::unique_ptr<RenderTarget> &&renderTarget) : device(device), renderTarget(
        std::move(renderTarget)) {

}

void RenderFrame::reset() {

}
