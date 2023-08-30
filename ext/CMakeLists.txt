add_subdirectory(tinyobjloader)

find_package(Vulkan REQUIRED)

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/src)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE Vulkan::Vulkan)

add_library(stb_image INTERFACE)
target_sources(stb_image INTERFACE)
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

set_target_properties(imgui PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
# Vulkan

add_subdirectory(glslang)

if (NOT TARGET glslang-default-resource-limits)
    add_library(glslang-default-resource-limits
            glslang/StandAlone/ResourceLimits.cpp)

    set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)

    target_include_directories(glslang-default-resource-limits
            PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
endif ()

if (NOT MSVC)
    target_compile_options(glslang PRIVATE
            "-Wno-logical-op-parentheses"
            "-Wno-unused-parameter")

    target_compile_options(SPIRV PRIVATE
            "-Wno-logical-op-parentheses"
            "-Wno-unused-parameter")
endif ()

set_target_properties(glslang PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(OGLCompiler PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(OSDependent PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(SPIRV PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)

# spirv-cross
add_subdirectory(spirv-cross)

set_target_properties(spirv-cross PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-core PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-glsl PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-cpp PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-hlsl PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-msl PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-reflect PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)
set_target_properties(spirv-cross-util PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)


#gltf loader
#add_subdirectory(tinygltf)

#add_library(tinygltf INTERFACE)
#target_sources(tinygltf INTERFACE tinygltf/tiny_gltf.h)
#target_include_directories(stb_image INTERFACE stb_image)
set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ktx)
add_library(ktx INTERFACE)

target_sources(ktx INTERFACE
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c)
target_include_directories(ktx INTERFACE ${KTX_DIR}/include ${KTX_DIR}/other_include)