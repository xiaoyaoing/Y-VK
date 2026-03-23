set(VIEWER_SOURCES
    main.cpp
    AppRegistry.cpp
    AppRegistry.h
)

add_executable(yvk_viewer ${VIEWER_SOURCES} ${shaders_src})
target_link_libraries(yvk_viewer
    PRIVATE
        framework
        yvk_render_module_raytracing
        yvk_render_module_vxgi
        yvk_render_module_pbr
)
target_include_directories(yvk_viewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
