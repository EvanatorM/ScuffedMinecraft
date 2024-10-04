cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

project(ScuffedMinecraft)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor/imgui)
add_subdirectory(src)
