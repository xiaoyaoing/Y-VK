function(yvk_add_render_module target_name)
    cmake_parse_arguments(MODULE "" "" "SOURCES;LINK_LIBS" ${ARGN})

    add_library(${target_name} STATIC ${MODULE_SOURCES})
    target_link_libraries(${target_name} PUBLIC framework ${MODULE_LINK_LIBS})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
endfunction()

file(GLOB_RECURSE YVK_RAYTRACING_MODULE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/raytracing/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/raytracing/*.cpp
)
yvk_add_render_module(yvk_render_module_raytracing SOURCES "${YVK_RAYTRACING_MODULE_SOURCES}")

file(GLOB_RECURSE YVK_VXGI_MODULE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/vxgi/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vxgi/*.cpp
)
yvk_add_render_module(yvk_render_module_vxgi SOURCES "${YVK_VXGI_MODULE_SOURCES}")

file(GLOB_RECURSE YVK_PBR_MODULE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/pbr/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pbr/*.cpp
)
yvk_add_render_module(yvk_render_module_pbr SOURCES "${YVK_PBR_MODULE_SOURCES}")
