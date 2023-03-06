#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VulkanMemoryAllocator::VulkanMemoryAllocator" for configuration "Debug"
set_property(TARGET VulkanMemoryAllocator::VulkanMemoryAllocator APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(VulkanMemoryAllocator::VulkanMemoryAllocator PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libVulkanMemoryAllocatord.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS VulkanMemoryAllocator::VulkanMemoryAllocator )
list(APPEND _IMPORT_CHECK_FILES_FOR_VulkanMemoryAllocator::VulkanMemoryAllocator "${_IMPORT_PREFIX}/lib/libVulkanMemoryAllocatord.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
