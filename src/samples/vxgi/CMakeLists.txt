get_filename_component(FOLDER_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} PATH)
get_filename_component(GRANDPARENT_DIR ${PARENT_DIR} PATH)


#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_VK_RESOURCES_DIR")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_MYSYMBOL")

add_executable(${FOLDER_NAME} vxgi.cpp vxgi.h ${shaders_src} ) 

target_link_libraries(${FOLDER_NAME} framework)
#target_include_directories(${FOLDER_NAME} PUBLIC "E:\\code\\VulkanFrameWorkLearn\\ext\\vma\\src")

