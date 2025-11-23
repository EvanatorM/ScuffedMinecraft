# Scuffed Minecraft

A very scuffed Minecraft clone. This project uses the following modules:
- [WillowVox Core Engine Module](https://github.com/EvanatorM/WV-Core)
- [WillowVox Voxel Worlds Engine Module](https://github.com/EvanatorM/WV-VoxelWorlds)

## Building

<details>
<summary>Installing dependencies on Debian-based distros</summary>
<br>

Run this command to get the dependencies for building GLFW:

```sh
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev cmake
```

</details>

<details>
<summary>Installing dependencies on Windows</summary>
<br>

You only need to have CMake installed on your system.

</details>

### Building with CMake

In the project root directory:
Create CMake files:

```sh
mkdir -p build
cd build
cmake ..
```

After that you can build the project using:

```sh
cmake --build .
```

Run the build command in the project root directory.

The final executable can be found at `(project root)/build/bin/(build type)`

#### Note for building with CMake

If you're running from a command line, make sure to run
the executable in the same directory as it is located
to ensure all resources are loaded properly.

## Contributing

As of right now, I'm not taking many contributions on this specific project because it is heavily tied to the YouTube series and because it is a learning experience. However, if you find some bugs or minor improvements that you want to make, you can do so and make a pull request. Don't get offended if I don't accept the changes, though. The other projects associated with this project (e.g. the core and voxel world modules) have their own contribution rules.