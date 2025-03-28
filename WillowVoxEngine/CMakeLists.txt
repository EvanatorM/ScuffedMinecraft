if (NOT DEFINED APP_TARGET)
    set(APP_TARGET "App")
endif()

# Ensure the target exists
if (NOT TARGET ${APP_TARGET})
    message(FATAL_ERROR "Target ${APP_TARGET} does not exist! Define APP_TARGET as the name of your application.")
endif()

# set variables based on build type
if (CMAKE_CONFIGURATION_TYPES)
    set(WILLOWVOX_DLL ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/WillowVoxEngine${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(WILLOWVOX_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/$<CONFIG>/WillowVoxEngine${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(WILLOWVOX_DLL ${CMAKE_CURRENT_SOURCE_DIR}/bin/debug/WillowVoxEngine${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(WILLOWVOX_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/debug/WillowVoxEngine${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(WILLOWVOX_DLL ${CMAKE_CURRENT_SOURCE_DIR}/bin/release/WillowVoxEngine${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(WILLOWVOX_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/release/WillowVoxEngine${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
endif()

set(WILLOWVOX_COPY_PATH $<TARGET_FILE_DIR:${APP_TARGET}>)

message("WILLOWVOX_COPY_PATH: ${WILLOWVOX_COPY_PATH}")
message("WILLOWVOX_DLL: ${WILLOWVOX_DLL}")
message("WILLOWVOX_LIB: ${WILLOWVOX_LIB}")

# Include directories
target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include     # WillowVox headers
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty  # WillowVox dependencies
)

# Link dependencies
target_link_libraries(${APP_TARGET} PRIVATE 
    ${WILLOWVOX_LIB}
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui${CMAKE_STATIC_LIBRARY_SUFFIX}
)

# Copy dynamic libraries to the output directory (Windows/Linux)
add_custom_target(copy_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${WILLOWVOX_DLL}
    ${WILLOWVOX_COPY_PATH}
)

# Add platform-specific preprocessor definitions
if(WIN32)
    target_compile_definitions(${APP_TARGET} PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${APP_TARGET} PUBLIC PLATFORM_MACOS)
elseif(UNIX)
    target_compile_definitions(${APP_TARGET} PUBLIC PLATFORM_LINUX)
else()
    message(FATAL_ERROR "Unknown platform!")
endif()