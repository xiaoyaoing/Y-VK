add_subdirectory(tinyobjloader)

#vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/src)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE vulkan)

add_library(stb_image INTERFACE)
target_sources(stb_image INTERFACE stb_image/stb_image.c)
target_include_directories(stb_image INTERFACE stb_image)



add_subdirectory(glfw)
add_subdirectory(spdlog)
add_subdirectory(glm)


# Vulkan
find_package(Vulkan REQUIRED)

