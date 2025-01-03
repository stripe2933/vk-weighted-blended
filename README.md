# vk-weighted-blended

![Build](https://github.com/stripe2933/vk-weighted-blended/actions/workflows/build.yml/badge.svg)

![Running screenshot](doc/images/running-screenshot.png)

Rendering both opaque and translucent objects using Weighted Blended OIT in Vulkan. In screenshot, there are 25 opaque and 100 translucent objects.

## Usage

This project requires support for C++20 modules and the C++23 standard library. The supported compiler is:
- Clang 18.1.2
- MSVC 19.40

Additionally, the following build tools are required:
- CMake 3.30
- Ninja 1.11

### Dependencies

This project depends on:
- [GLFW](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- My own Vulkan-Hpp helper library, [vku](https://github.com/stripe2933/vku/tree/module) (branch `module`), which has the following dependencies:
  - [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp)
  - [VulkanMemoryAllocator-Hpp](https://github.com/YaaZ/VulkanMemoryAllocator-Hpp)

Dependencies will be automatically fetched using vcpkg.

### Build Steps

> [!TIP]
> This project uses GitHub Runner to ensure build compatibility on Windows (with MSVC), macOS and Linux (with Clang). You can check the workflow files in the [.github/workflows](.github/workflows) folder.

This project, along with its dependency `vku`, supports vcpkg for dependency management. Follow these steps to build the project:

```sh
git clone https://github.com/stripe2933/vk-weighted-blended
cd vk-weighted-blended
cmake --preset=default # Or use your own configuration preset that inherits from the "default" preset.
cmake --build build -t vk-weighted-blended
```

The executable will be located in the `build` folder.

### Shader compilation

All shaders are located in the [shaders](/shaders) folder. They are automatically compiled to SPIR-V and embedded into the binary executable during the build.

### Data Flow Visualization

![Data flow](doc/images/data-flow.png)

Screenshot from Xcode Metal Debugger.

Note that this application uses 4x MSAA, therefore each color attachments have their own resolve attachments, and they are not stored for the next subpass usage.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE.txt) file for details.