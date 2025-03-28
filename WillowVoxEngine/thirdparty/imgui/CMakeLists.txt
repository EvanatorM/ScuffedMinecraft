add_library(imgui STATIC
  imgui.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_impl_glfw.cpp
  imgui_impl_opengl3.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/external/
)

# Set output properties for the library
set_target_properties(imgui PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)