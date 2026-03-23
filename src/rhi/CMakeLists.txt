find_package(Vulkan REQUIRED)

set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../shaders")
message("SHADER_DIR: ${SHADER_DIR}")

function(yvk_setup_target target_name)
    target_compile_definitions(${target_name} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../app
        ${CMAKE_CURRENT_SOURCE_DIR}/../engine
        ${CMAKE_CURRENT_SOURCE_DIR}/../rendercore
        ${SHADER_DIR}
        $<BUILD_INTERFACE:${Y_VK_THIRD_PARTY_DIR}/DirectXShaderCompiler/include>
    )
endfunction()

file(GLOB_RECURSE YVK_PLATFORM_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/PlatForm/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PlatForm/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/BufferPool.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/BufferPool.cpp
)

add_library(yvk_platform STATIC ${YVK_PLATFORM_SOURCES})
yvk_setup_target(yvk_platform)
target_link_libraries(yvk_platform PUBLIC
    glfw
    glm
    spdlog
    vma
    stb_image
    imgui
    ktx
    astc
    volk
    ctpl
    nlohmann_json
    tiny_obj_loader
    glslang
    glslang-default-resource-limits
    SPIRV
    spirv-cross-core
    spirv-cross-cpp
    spirv-cross-glsl
    spirv-cross-reflect
    dxc
    Vulkan::Vulkan
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${YVK_PLATFORM_SOURCES})

file(GLOB_RECURSE YVK_RHI_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/samplerCPP/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/samplerCPP/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Descriptor/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Descriptor/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Device/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Device/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Images/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Images/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/RayTracing/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/RayTracing/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Shader/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Shader/*.cpp
)
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/BufferPool\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/View\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/Transform\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/CameraComponent\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/RenderContext\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/FrameBuffer\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/RenderPass\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/RenderTarget\\.(h|cpp)$")
list(FILTER YVK_RHI_SOURCES EXCLUDE REGEX "/ResourceBindingState\\.(h|cpp)$")

add_library(yvk_rhi STATIC ${YVK_RHI_SOURCES})
yvk_setup_target(yvk_rhi)
target_link_libraries(yvk_rhi PUBLIC
    yvk_platform
    spdlog
    vma
    stb_image
    glm
    imgui
    ktx
    astc
    volk
    ctpl
    nlohmann_json
    tiny_obj_loader
    glslang
    glslang-default-resource-limits
    SPIRV
    spirv-cross-core
    spirv-cross-cpp
    spirv-cross-glsl
    spirv-cross-reflect
    dxc
    Vulkan::Vulkan
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${YVK_RHI_SOURCES})





