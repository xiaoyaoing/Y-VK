function(yvk_setup_rendercore_target target_name)
    target_compile_definitions(${target_name} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${SHADER_DIR}
        $<BUILD_INTERFACE:${Y_VK_THIRD_PARTY_DIR}/DirectXShaderCompiler/include>
    )
endfunction()

file(GLOB_RECURSE YVK_RENDERGRAPH_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderGraph/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderGraph/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/FrameBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/FrameBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderPass.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderPass.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderTarget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderTarget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/ResourceBindingState.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/ResourceBindingState.cpp
)

add_library(yvk_rendergraph STATIC ${YVK_RENDERGRAPH_SOURCES})
yvk_setup_rendercore_target(yvk_rendergraph)
target_link_libraries(yvk_rendergraph PUBLIC yvk_scene yvk_rhi)

file(GLOB_RECURSE YVK_RENDERCORE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PostProcess/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderIng/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderIng/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderPasses/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/RenderPasses/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/RenderContext.cpp
)

add_library(yvk_rendercore STATIC ${YVK_RENDERCORE_SOURCES})
yvk_setup_rendercore_target(yvk_rendercore)
target_link_libraries(yvk_rendercore PUBLIC yvk_rendergraph yvk_scene yvk_engine yvk_rhi)
