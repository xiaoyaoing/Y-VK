
find_package(Vulkan REQUIRED)

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma/src)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE Vulkan::Vulkan)

add_library(stb_image INTERFACE)
set(STB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stb_image)
target_sources(stb_image INTERFACE  ${STB_DIR}/stb_image_resize.h )
target_include_directories(stb_image INTERFACE ${STB_DIR})

add_subdirectory(glfw)
add_subdirectory(spdlog)


add_subdirectory(glm)
set(GLM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
target_include_directories(glm INTERFACE ${GLM_DIR})

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_demo.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
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

if(NOT TARGET glslang-default-resource-limits)
        add_library(glslang-default-resource-limits
                glslang/StandAlone/ResourceLimits.cpp)

        set_target_properties(glslang-default-resource-limits PROPERTIES FOLDER "EXT" POSITION_INDEPENDENT_CODE ON)

        target_include_directories(glslang-default-resource-limits
                PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glslang)
endif()

if(NOT MSVC)
        target_compile_options(glslang PRIVATE
                "-Wno-logical-op-parentheses"
                "-Wno-unused-parameter")

        target_compile_options(SPIRV PRIVATE
                "-Wno-logical-op-parentheses"
                "-Wno-unused-parameter")
endif()

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

set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ktx)
add_library(ktx INTERFACE)

target_sources(ktx INTERFACE
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c)
target_include_directories(ktx INTERFACE ${KTX_DIR}/include ${KTX_DIR}/other_include ${KTX_DIR}/lib)

# astc
set(ASTC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/astc)
set(ASTC_INCLUDE_DIR ${ASTC_DIR}/Source)
set(ASTC_SOURCE_DIR ${ASTC_DIR}/Source)

set(ASTC_SOURCES
        ${ASTC_SOURCE_DIR}/astc_image_load_store.cpp
        ${ASTC_SOURCE_DIR}/astc_pick_best_endpoint_format.cpp
        ${ASTC_SOURCE_DIR}/astc_color_quantize.cpp
        ${ASTC_SOURCE_DIR}/astc_weight_align.cpp
        ${ASTC_SOURCE_DIR}/astc_integer_sequence.cpp
        ${ASTC_SOURCE_DIR}/astc_ideal_endpoints_and_weights.cpp
        ${ASTC_SOURCE_DIR}/astc_find_best_partitioning.cpp
        ${ASTC_SOURCE_DIR}/astc_compress_symbolic.cpp
        ${ASTC_SOURCE_DIR}/astc_decompress_symbolic.cpp
        ${ASTC_SOURCE_DIR}/astc_symbolic_physical.cpp
        ${ASTC_SOURCE_DIR}/astc_toplevel.cpp
        ${ASTC_SOURCE_DIR}/astc_stb_tga.cpp
        ${ASTC_SOURCE_DIR}/softfloat.cpp
        ${ASTC_SOURCE_DIR}/mathlib.cpp
        ${ASTC_SOURCE_DIR}/astc_quantization.cpp
        ${ASTC_SOURCE_DIR}/astc_block_sizes2.cpp
        ${ASTC_SOURCE_DIR}/astc_percentile_tables.cpp
        ${ASTC_SOURCE_DIR}/astc_averages_and_directions.cpp
        ${ASTC_SOURCE_DIR}/astc_partition_tables.cpp
        ${ASTC_SOURCE_DIR}/astc_color_unquantize.cpp
        ${ASTC_SOURCE_DIR}/astc_encoding_choice_error.cpp
        ${ASTC_SOURCE_DIR}/astc_kmeans_partitioning.cpp
        ${ASTC_SOURCE_DIR}/astc_weight_quant_xfer_tables.cpp
        ${ASTC_SOURCE_DIR}/astc_compute_variance.cpp
        ${ASTC_SOURCE_DIR}/astc_ktx_dds.cpp
)

add_library(astc STATIC ${ASTC_SOURCES})
target_include_directories(astc PUBLIC ${ASTC_INCLUDE_DIR})
target_compile_definitions(astc PRIVATE -DNO_STB_IMAGE_IMPLEMENTATION)
set_target_properties(astc PROPERTIES FOLDER "ext" POSITION_INDEPENDENT_CODE ON)

# target_link_libraries(astc PUBLIC stb)
add_library(ctpl INTERFACE)
set(CTPL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CTPL)
target_sources(ctpl INTERFACE ${CTPL_DIR}/ctpl_stl.h)
target_include_directories(ctpl INTERFACE ${CTPL_DIR})

add_subdirectory(volk)
