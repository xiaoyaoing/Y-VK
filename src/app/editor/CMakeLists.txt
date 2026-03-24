set(EDITOR_SOURCES
    main.cpp
    EditorApplication.cpp
    EditorApplication.h
    Renderer.h
    RendererRegistry.cpp
    RendererRegistry.h
)

add_executable(yvk_editor ${EDITOR_SOURCES} ${shaders_src})
target_link_libraries(yvk_editor
    PRIVATE
        framework
        yvk_render_module_raytracing
        yvk_render_module_vxgi
        yvk_render_module_pbr
)
target_include_directories(yvk_editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)
