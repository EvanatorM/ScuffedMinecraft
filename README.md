# Scuffed Minecraft
A Minecraft clone made in C++ and OpenGL

## How to Run
To run the game, download the ScuffedMinecraft zip file from the [latest release](https://github.com/EvanatorM/ScuffedMinecraft/releases/latest), unzip the file, and run ScuffedMinecraft.exe. The assets folder must be in the same place as the exe file.

## Building

### Building with Visual Studio
Import the project in Visual Studio 17 or higher and build it.

### Building with CMake
In the project root directory:
Create CMake files:
```sh
mkdir -p build
cd build
cmake ../ScuffedMinecraft
```
After that you can build the project using:
```sh
cmake --build ./build
```
Run the build command in the project root directory.

The final executable can be found at `(project root)/ScuffedMinecraft/bin`

#### Note for building with CMake
If you're running from a command line, make sure to run
the executable in the same directory as it is located
to ensure all resources are loaded properly.
