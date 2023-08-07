//
// Created by 打工人 on 2023/3/28.
//

#include "RenderFrame.h"

CommandBuffer &
RenderFrame::requestCommandBuffer(const Queue &queue, CommandBuffer::ResetMode reset_mode, VkCommandBufferLevel level) {
    return <#initializer#>;
}

RenderTarget &RenderFrame::getRenderTarget() {
    return <#initializer#>;
}

RenderFrame::RenderFrame(Device &device, std::unique_ptr<RenderTarget> &&renderTarget) {

}

void RenderFrame::reset() {

}
