/**
 * @file 
 * @author JunPing Yuan
 * @brief 
 * @version 0.1
 * @date 2023/3/3
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Vulkan.h"
#include <memory>

class Buffer;

class CommandBuffer;

class FrameBuffer;

class Pipeline;

class RenderPass;

class Device;

using BufferPtr = std::shared_ptr<Buffer>;
using CommandBufferPtr = std::shared_ptr<CommandBuffer>;
using RenderPassBasePtr = std::shared_ptr<RenderPass>;
using FramebufferPtr = std::shared_ptr<FrameBuffer>;
using PipelineBasePtr = std::shared_ptr<Pipeline>;
using DevicePtr = std::shared_ptr<Device>;

//template<class T>
//class Base {
//
//};