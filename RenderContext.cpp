//
// Created by 打工人 on 2023/3/30.
//
#include <Engine/RenderContext.h>
#include "Engine/CommandBuffer.h"
#include <Engine/Device.h>
#include <Engine/SwapChain.h>

RenderContext::RenderContext(ptr<Device> device, VkSurfaceKHR surface, ptr<Window> window) {

}

CommandBuffer RenderContext::begin() {
    assert(prepared && "RenderContext not prepared for rendering, call prepare()");

    if (!frameActive)
    {
        beginFrame();
    }

    if (acquiredSem == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Couldn't begin frame");
    }
    return getActiveRenderFrame().requestCommandBuffer();
}

void RenderContext::beginFrame() {
    //*
    auto & prevFrame = _frames[activeFrameIndex];
    acquiredSem = prevFrame.requestSemOwnerShip();
    auto result = swapChain->acquireNextImage(activeFrameIndex,acquiredSem,VK_NULL_HANDLE);
    if(result!=VK_SUCCESS){
        return;
    }
    frameActive = true;
    waitFrame();
}

void RenderContext::waitFrame() {

}

RenderFrame &RenderContext::getActiveRenderFrame() {
    assert(activeFrameIndex<_frames.size());
    return _frames[activeFrameIndex];
}
