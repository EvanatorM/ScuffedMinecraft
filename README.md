# Scuffed Minecraft

A very scuffed Minecraft clone for my YouTube channel.

## Building

<details>
<summary>Installing dependencies on Debian-based distros</summary>
<br>

Run this command to get the dependencies for building GLFW:

```sh
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev cmake
```

Follow this guide to install the Vulkan SDK: [Getting Started with the Linux Tarball Vulkan SDK](https://vulkan.lunarg.com/doc/view/latest/linux/getting_started.html)

</details>

<details>
<summary>Installing dependencies on Windows</summary>
<br>

You need to have [CMake](https://cmake.org/download/) and the [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) installed.

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

As of right now, I'm not taking many contributions on this specific project because it is heavily tied to the YouTube series and because it is a learning experience. However, if you find some bugs or minor improvements that you want to make, you can do so and make a pull request. Don't get offended if I don't accept the changes, though.