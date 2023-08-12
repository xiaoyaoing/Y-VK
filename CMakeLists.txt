cmake_minimum_required(VERSION 3.17)
project(VulkanDemo)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ext)
add_subdirectory(framework)
add_subdirectory(samples/sample1)
add_dependencies(sample1 framework)


file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


