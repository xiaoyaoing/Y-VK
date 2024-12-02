cmake_minimum_required(VERSION 3.25)

project(Y-VK)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DVK_RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources/\")
add_definitions(-DVK_SHADERS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\")

file(GLOB_RECURSE shaders_src CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rgen
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rahit
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rchit
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rmiss
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rcall
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rint
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/tinygltf)

add_subdirectory(ext)
add_subdirectory(src/framework)

add_subdirectory(src/RayTracer)
add_subdirectory(src/samples/renderGraphTest)
add_subdirectory(src/samples/computePipelineTest)
add_subdirectory(src/samples/vxgi)
add_subdirectory(src/samples/pbrLab)
add_subdirectory(src/samples/ssgi)
add_subdirectory(src/samples/meshShaderGrass)
add_subdirectory(src/samples/triangle)
add_subdirectory(src/samples/tinynanite)
#add_subdirectory(src/samples/visualize_texture)

# add_subdirectory(src/samples/vxgiapp)
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
