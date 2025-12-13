include(FetchContent)

macro(_fetch_git_project NAME REPO COMMIT)
  message(STATUS "Module: ${NAME} @ ${COMMIT}")
  FetchContent_Declare(${NAME}
    GIT_REPOSITORY ${REPO}
    GIT_TAG        ${COMMIT}      # exact commit from lockfile
    GIT_SHALLOW    TRUE
    UPDATE_DISCONNECTED FALSE
  )
  FetchContent_MakeAvailable(${NAME})
endmacro()

# Add engine modules
_fetch_git_project(WVCore https://github.com/EvanatorM/WV-Core.git alpha-v0.3.0)
target_link_libraries(${PROJECT_NAME} PUBLIC WVCore)
target_include_directories(${PROJECT_NAME} PUBLIC ${wvcore_SOURCE_DIR}/include)

_fetch_git_project(WVVoxelWorlds https://github.com/EvanatorM/WV-VoxelWorlds.git alpha-v0.2.0)
target_link_libraries(${PROJECT_NAME} PRIVATE WVVoxelWorlds)
target_include_directories(${PROJECT_NAME} PUBLIC ${wvvoxelworlds_SOURCE_DIR}/include)

# Add ImGUI
_fetch_git_project(imgui https://github.com/ocornut/imgui.git v1.92.4)

set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/misc/cpp
  ${imgui_SOURCE_DIR}/backends
)

add_library(imgui::imgui ALIAS imgui)
set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(IMGUI_BACKEND_DIR ${imgui_SOURCE_DIR}/backends)

add_library(imgui_backend
  ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
  ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backend PUBLIC
  ${IMGUI_BACKEND_DIR}
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/misc/cpp
)

find_package(OpenGL REQUIRED)
target_link_libraries(imgui_backend PUBLIC
  imgui::imgui
  glfw
  OpenGL::GL
)

target_link_libraries(${PROJECT_NAME} PRIVATE imgui_backend)
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_backend_SOURCE_DIR})

# Add FastNoiseLite
_fetch_git_project(fastnoise https://github.com/Auburn/FastNoiseLite.git v1.1.1)
target_include_directories(${PROJECT_NAME} PRIVATE ${fastnoise_SOURCE_DIR}/Cpp)