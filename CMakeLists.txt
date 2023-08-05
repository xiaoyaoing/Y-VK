cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)


# 检查环境变量
#include_directories(${PROJECT_SOURCE_DIR})

#add_library(framework SHARED framework/vk.cpp framework/VertexData.h framework/Vulkan.h framework/Buffer.cpp framework/Buffer.h framework/CommandBuffer.cpp framework/CommandBuffer.h framework/ForwardDeclation.h framework/FrameBuffer.cpp framework/FrameBuffer.h framework/Pipeline.cpp framework/Pipeline.h framework/Device.cpp framework/Device.h framework/RenderPass.cpp framework/RenderPass.h
#        framework/Descriptor/DescriptorPool.cpp framework/Descriptor/DescriptorSet.cpp framework/Descriptor/DescriptorSet.h framework/Descriptor/DescriptorLayout.cpp framework/Descriptor/DescriptorLayout.h ext/stb_image/stb_image.c framework/Images/Image.cpp framework/Images/Image.h framework/Command/CommandPool.cpp framework/Queue.cpp framework/Queue.h framework/Sync/Fence.cpp framework/Images/ImageView.cpp framework/Images/ImageView.h framework/Images/Sampler.cpp framework/Images/Sampler.h
#        framework/Utils/DebugUtils.cpp framework/Window.cpp framework/Window.h framework/Instance.cpp framework/Instance.h framework/SwapChain.cpp framework/SwapChain.h framework/Mesh.cpp framework/Mesh.h framework/App/Application.cpp framework/App/Application.h framework/Common/String.cpp framework/Common/String.h framework/RenderContext.h framework/Common/Log.cpp framework/Common/Log.h RenderTarget.cpp RenderTarget.h framework/Utils/VkUtils.h RenderFrame.cpp RenderFrame.h RenderContext.cpp)
#
#
## GLM
#include_directories(ext/glm)
#include_directories(ext/vma)
#
## Vulkan
#find_package(Vulkan REQUIRED FATAL_ERROR)
#
#target_include_directories(framework PUBLIC ${Vulkan_INCLUDE_DIRS} ext/glfw/include ext/spdlog/include)
#
add_subdirectory(ext)
#
#include_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/include/)
#link_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/lib/libMoltenVK.dylib)
#target_link_libraries(framework tinyobjloader glfw Vulkan::Vulkan vma spdlog::spdlog)
add_subdirectory(framework)
add_subdirectory(samples/test)
add_dependencies(test framework)


file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


