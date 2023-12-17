file(GLOB_RECURSE rt_src ${CMAKE_CURRENT_SOURCE_DIR}/*.h 
${CMAKE_CURRENT_SOURCE_DIR}/*.cpp) 
add_executable(RTRT ${rt_src} ${shaders_src})

target_link_libraries(RTRT framework)