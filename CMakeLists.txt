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

set(BINARY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/target")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_ROOT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_ROOT_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_ROOT_DIR}/lib)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${BINARY_ROOT_DIR}/bin)
set(Y_VK_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext")
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ext/tinygltf)
add_subdirectory(ext)
add_subdirectory(src/rhi)
add_subdirectory(src/engine)
add_subdirectory(src/rendercore)
add_subdirectory(src/render_features)
add_subdirectory(src/render_modules)
add_subdirectory(src/app)

file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
