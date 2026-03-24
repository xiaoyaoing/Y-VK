function(yvk_add_render_feature target_name)
    cmake_parse_arguments(FEATURE "" "" "SOURCES;LINK_LIBS" ${ARGN})

    add_library(${target_name} STATIC ${FEATURE_SOURCES})
    target_link_libraries(${target_name} PUBLIC framework ${FEATURE_LINK_LIBS})
    target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
endfunction()

yvk_add_render_feature(yvk_render_feature_pbr
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/pbr/PbrFeatureSet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pbr/PbrFeatureSet.cpp
)

yvk_add_render_feature(yvk_render_feature_vxgi
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/vxgi/VxgiFeatureSet.h
        ${CMAKE_CURRENT_SOURCE_DIR}/vxgi/VxgiFeatureSet.cpp
)
