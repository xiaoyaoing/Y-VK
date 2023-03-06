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
class Framebuffer;
class PipelineBase;
class RenderPass;
class Device;

using BufferPtr = std::shared_ptr<Buffer>;
using CommandBufferPtr = std::shared_ptr<CommandBuffer>;
using RenderPassBasePtr = std::shared_ptr<RenderPass>;
using FramebufferPtr = std::shared_ptr<Framebuffer>;
using PipelineBasePtr = std::shared_ptr<PipelineBase>;
using DevicePtr = std::shared_ptr<Device>;

//template<class T>
//class Base {
//
//};