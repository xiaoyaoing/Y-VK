

add_executable(RTRT RayTracer.cpp RayTracer.h ${shaders_src})

target_link_libraries(RTRT framework)