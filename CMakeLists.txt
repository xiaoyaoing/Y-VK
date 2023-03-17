cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)


# 检查环境变量
include_directories(${PROJECT_SOURCE_DIR})

add_executable(VulkanDemo vk.cpp VertexData.h Vulkan.h Buffer.cpp Buffer.h CommandBuffer.cpp CommandBuffer.h ForwardDeclation.h FrameBuffer.cpp FrameBuffer.h PipelineBase.cpp PipelineBase.h Device.cpp Device.h RenderPass.cpp RenderPass.h
        Descriptor/DescriptorPool.cpp Descriptor/DescriptorSet.cpp Descriptor/DescriptorSet.h Descriptor/DescriptorLayout.cpp Descriptor/DescriptorLayout.h ext/stb_image/stb_image.c Images/Image.cpp Images/Image.h Command/CommandPool.cpp Queue.cpp Queue.h Sync/Fence.cpp Images/ImageView.cpp Images/ImageView.h Images/Sampler.cpp Images/Sampler.h
        Utils/DebugUtils.cpp Window.cpp Window.h Instance.cpp Instance.h Application.cpp Application.h SwapChain.cpp SwapChain.h)

# GLFW
add_library(glfw SHARED IMPORTED Utils/DebugUtils.cpp Utils/DebugUtils.h)
SET_TARGET_PROPERTIES(glfw PROPERTIES IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.3.dylib" "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.3.3.dylib.dylib"
        "${PROJECT_SOURCE_DIR}/ext/glfw/lib/libglfw.dylib")

# GLM
include_directories(ext/glm)
include_directories(ext/VulkanMemoryAllocator)

# Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext/VulkanMemoryAllocator)
target_link_libraries(${PROJECT_NAME} VulkanMemoryAllocator)

include_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/include/)
link_directories(/Users/yjp/VulkanSDK/1.3.231.1/macOS/lib/libMoltenVK.dylib)




file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
