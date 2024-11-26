add_library(metis INTERFACE)
target_include_directories(metis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(metis INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib/metis.lib)