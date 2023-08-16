add_subdirectory(tinyobjloader)

find_package(Vulkan REQUIRED)

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/src)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE Vulkan::Vulkan)

add_library(stb_image INTERFACE)
target_sources(stb_image INTERFACE stb_image/stb_image.c)
target_include_directories(stb_image INTERFACE stb_image)

add_subdirectory(glfw)
add_subdirectory(spdlog)
add_subdirectory(glm)


# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imconfig.h"
        "${IMGUI_DIR}/imgui.h"
        "${IMGUI_DIR}/imgui_internal.h"
        "${IMGUI_DIR}/imstb_rectpack.h"
        "${IMGUI_DIR}/imstb_textedit.h"
        "${IMGUI_DIR}/imstb_truetype.h")

add_library(imgui STATIC ${IMGUI_FILES})

target_include_directories(imgui PUBLIC ${IMGUI_DIR})

set_target_properties(imgui PROPERTIES FOLDER "ThirdParty" POSITION_INDEPENDENT_CODE ON)
# Vulkan
