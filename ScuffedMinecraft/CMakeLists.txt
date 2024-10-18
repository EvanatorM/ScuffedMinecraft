cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(ScuffedMinecraft)

# output directories
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# copy assets
add_custom_target(copy_assets ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/assets
  ${CMAKE_BINARY_DIR}/assets
  COMMENT "Copying assets")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# global dependency directories
include_directories(../Dependencies/include/)
link_directories(../Dependencies/lib/)

add_subdirectory(vendor/imgui)
add_subdirectory(src)
