get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
get_filename_component(GRANDPARENT_DIR ${PARENT_DIR} PATH)

add_executable(${FOLDER_NAME} test.cpp Demo.cpp Demo.h)

target_link_libraries(${FOLDER_NAME} framework)
