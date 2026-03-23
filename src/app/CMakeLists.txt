add_library(framework INTERFACE)
target_link_libraries(framework INTERFACE yvk_rendercore yvk_engine yvk_scene yvk_rhi yvk_platform)
target_include_directories(framework INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine
    ${CMAKE_CURRENT_SOURCE_DIR}/../rendercore
    ${SHADER_DIR}
    $<BUILD_INTERFACE:${Y_VK_THIRD_PARTY_DIR}/DirectXShaderCompiler/include>
)

set(FRAMEWORK_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Path to the framework include directory")

add_subdirectory(viewer)
