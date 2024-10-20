add_executable(scuffed_mc
  Application.cpp Chunk.cpp Shader.cpp 
  Block.cpp NoiseSettings.cpp SurfaceFeature.cpp 
  Camera.cpp Planet.cpp WorldGen.cpp ../vendor/glad.c
)

if(LINUX)
  add_compile_definitions(LINUX)
endif()

# make sure the program runs in the right place from visual studio
set_target_properties(scuffed_mc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

target_link_libraries(scuffed_mc imgui $<IF:$<PLATFORM_ID:Windows>,glfw3,glfw>)
