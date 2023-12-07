cmake_minimum_required(VERSION 3.17)

project(VulkanDemo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DVK_RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources/\")
add_definitions(-DVK_SHADERS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\")

add_subdirectory(ext)
add_subdirectory(src/framework)
add_subdirectory(src/RayTracer)

#add_subdirectory(src/samples/sample1)
#add_subdirectory(src/samples/instancing)
#add_subdirectory(src/samples/defered)
#add_subdirectory(src/samples/shaderreFlection)
add_subdirectory(src/samples/renderGraphTest)
add_subdirectory(src/samples/computePipelineTest)

file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
