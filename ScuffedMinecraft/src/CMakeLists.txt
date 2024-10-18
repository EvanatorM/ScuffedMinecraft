add_executable(scuffed_mc
  Application.cpp Chunk.cpp Shader.cpp 
  Block.cpp NoiseSettings.cpp SurfaceFeature.cpp 
  Camera.cpp Planet.cpp WorldGen.cpp ../vendor/glad.c
) 

target_link_libraries(scuffed_mc imgui glfw3)
