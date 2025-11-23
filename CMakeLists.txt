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
endif()

# Copy assets folder to build folder
add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Copying assets..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:ScuffedMinecraft>/assets
)

target_compile_definitions(ScuffedMinecraft PUBLIC
$<$<CONFIG:Debug>:DEBUG_MODE>
$<$<CONFIG:Release>:RELEASE_MODE>
)