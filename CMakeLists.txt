cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DVK_RESOURCES_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources/\")
add_definitions(-DVK_SHADERS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\")

add_subdirectory(ext)
add_subdirectory(framework)


add_subdirectory(samples/sample1)
add_dependencies(sample1 framework)


file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


