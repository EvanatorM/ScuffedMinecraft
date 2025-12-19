cmake_minimum_required(VERSION 3.24)
project(ScuffedMinecraft LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ScuffedMinecraft 
    src/main.cpp
    
    src/ui/UIManager.cpp

    src/world/ScuffedWorldGen.cpp
)

target_include_directories(ScuffedMinecraft PRIVATE include)

set_target_properties(ScuffedMinecraft PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

include(${CMAKE_SOURCE_DIR}/cmake/modules.cmake OPTIONAL)

target_compile_definitions(ScuffedMinecraft
    PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
)

if (MSVC)
    add_compile_options(/fsanitize=address /Zi /Od)
    add_link_options(/fsanitize=address)
else()
    target_compile_options(ScuffedMinecraft PRIVATE -Wall -Wextra)
endif()

target_compile_definitions(ScuffedMinecraft PUBLIC
$<$<CONFIG:Debug>:DEBUG_MODE>
$<$<CONFIG:Release>:RELEASE_MODE>
)

add_custom_command(
  TARGET ScuffedMinecraft POST_BUILD  # Trigger after building MyExe
  COMMAND ${CMAKE_COMMAND} -E copy_directory  # Copy only if DLL has changed
    ${CMAKE_SOURCE_DIR}/assets  # Source: Full path to MyDLL.dll
    $<TARGET_FILE_DIR:ScuffedMinecraft>/assets  # Destination: MyExe's directory
  COMMENT "Copying assets"  # Log message
)
