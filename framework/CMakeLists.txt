
file(GLOB_RECURSE PROJECT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*")
file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
set(target_name framework)

message("Project Files: ${HEADER_FILES}")

# 接下来你可以使用source_group命令将这些文件分组，以在IDE中更好地组织项目结构
#source_group("Project Files" FILES ${PROJECT_FILES})

add_library(${target_name} STATIC ${PROJECT_FILES} Subpass.cpp Subpass.h Vulkan.cpp ResourceCache.cpp ResourceCache.h)
message(${target_name})
find_package(Vulkan REQUIRED)
target_link_libraries(${target_name} PUBLIC
        spdlog
        vma
        tinyobjloader
        glfw
        stb_image
        glm
        Vulkan::Vulkan)

message(STATUS ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories("C:\\VulkanSDK\\1.3.250.1\\Include")

# 设置头文件搜索路径
target_include_directories(${target_name} PUBLIC ${PROJECT_SOURCE_DIR}/framework "C:\\VulkanSDK\\1.3.250.1\\Include")

set(FRAMEWORK_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/framework" CACHE PATH "Path to the framework include directory")

# 获取 framework 文件夹下的所有源文件
file(GLOB_RECURSE FRAMEWORK_FILES ${PROJECT_SOURCE_DIR}/framework/*.h)

# 设置源文件
target_sources(${target_name} PRIVATE ${FRAMEWORK_FILES})