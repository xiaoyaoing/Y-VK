function(yvk_setup_engine_target target_name)
    target_compile_definitions(${target_name} PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
    target_include_directories(${target_name} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${SHADER_DIR}
        $<BUILD_INTERFACE:${Y_VK_THIRD_PARTY_DIR}/DirectXShaderCompiler/include>
    )
endfunction()

file(GLOB_RECURSE YVK_SCENE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Compoments/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Compoments/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Images/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/Images/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/SceneLoader/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/SceneLoader/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Scene/SceneLoader/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/InputEvent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/InputEvent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/imfilebrowser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/View.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/View.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/Transform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/Transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/CameraComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../rhi/Core/CameraComponent.cpp
)

add_library(yvk_scene STATIC ${YVK_SCENE_SOURCES})
yvk_setup_engine_target(yvk_scene)
target_link_libraries(yvk_scene PUBLIC yvk_rhi)

file(GLOB_RECURSE YVK_ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/Gui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Gui/Gui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/IO/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/IO/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SDF/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SDF/*.cpp`r`n    ${CMAKE_CURRENT_SOURCE_DIR}/../app/App/*.h`r`n    ${CMAKE_CURRENT_SOURCE_DIR}/../app/App/*.cpp
)

add_library(yvk_engine STATIC ${YVK_ENGINE_SOURCES})
yvk_setup_engine_target(yvk_engine)
target_link_libraries(yvk_engine PUBLIC yvk_scene yvk_rhi)

