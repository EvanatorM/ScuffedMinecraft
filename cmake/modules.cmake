include(FetchContent)

macro(_add_engine_git_module NAME REPO COMMIT)
  message(STATUS "Module: ${NAME} @ ${COMMIT}")
  FetchContent_Declare(${NAME}
    GIT_REPOSITORY ${REPO}
    GIT_TAG        ${COMMIT}      # exact commit from lockfile
    GIT_SHALLOW    TRUE
    UPDATE_DISCONNECTED FALSE
  )
  FetchContent_MakeAvailable(${NAME})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${NAME})
  target_include_directories(${PROJECT_NAME} PRIVATE ${${NAME}_SOURCE_DIR}/include)
endmacro()

macro(_fetch_git_module NAME REPO COMMIT)
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
_add_engine_git_module(WVCore https://github.com/EvanatorM/WV-Core.git alpha-v0.2.3)

# Add ImGUI
_fetch_git_module(imgui https://github.com/ocornut/imgui.git v1.92.4)

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
_fetch_git_module(fastnoise https://github.com/Auburn/FastNoiseLite.git v1.1.1)
target_include_directories(${PROJECT_NAME} PRIVATE ${fastnoise_SOURCE_DIR}/Cpp)