cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)


# 检查环境变量
include_directories(${PROJECT_SOURCE_DIR})

add_library(Engine SHARED Engine/vk.cpp Engine/VertexData.h Engine/Vulkan.h Engine/Buffer.cpp Engine/Buffer.h Engine/CommandBuffer.cpp Engine/CommandBuffer.h Engine/ForwardDeclation.h Engine/FrameBuffer.cpp Engine/FrameBuffer.h Engine/Pipeline.cpp Engine/Pipeline.h Engine/Device.cpp Engine/Device.h Engine/RenderPass.cpp Engine/RenderPass.h
        Engine/Descriptor/DescriptorPool.cpp Engine/Descriptor/DescriptorSet.cpp Engine/Descriptor/DescriptorSet.h Engine/Descriptor/DescriptorLayout.cpp Engine/Descriptor/DescriptorLayout.h ext/stb_image/stb_image.c Engine/Images/Image.cpp Engine/Images/Image.h Engine/Command/CommandPool.cpp Engine/Queue.cpp Engine/Queue.h Engine/Sync/Fence.cpp Engine/Images/ImageView.cpp Engine/Images/ImageView.h Engine/Images/Sampler.cpp Engine/Images/Sampler.h
        Engine/Utils/DebugUtils.cpp Engine/Window.cpp Engine/Window.h Engine/Instance.cpp Engine/Instance.h Engine/SwapChain.cpp Engine/SwapChain.h Engine/Mesh.cpp Engine/Mesh.h Engine/App/Application.cpp Engine/App/Application.h Engine/Common/String.cpp Engine/Common/String.h Engine/RenderContext.h Engine/Common/Log.cpp Engine/Common/Log.h RenderTarget.cpp RenderTarget.h Engine/Utils/VkUtils.h RenderFrame.cpp RenderFrame.h RenderContext.cpp)

# GLFW
add_library(glfw SHARED IMPORTED Engine/Utils/DebugUtils.cpp Engine/Utils/DebugUtils.h)
SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.3.dylib" "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.3.3.dylib.dylib"
        "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.dylib")

# GLM
include_directories(ext/glm)
include_directories(ext/VulkanMemoryAllocator)

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
target_include_directories(Engine PUBLIC ${Vulkan_INCLUDE_DIRS})

add_subdirectory(ext)

include_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/include/)
link_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/lib/libMoltenVK.dylib)
target_link_libraries(Engine tinyobjloader glfw Vulkan::Vulkan VulkanMemoryAllocator)

add_subdirectory(samples/test)
target_link_libraries(test Engine)
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


