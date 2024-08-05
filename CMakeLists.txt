cmake_minimum_required(VERSION 3.30)

# Enable CMake's experimental standard library module support.
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "0e5b6991-d74f-4b3d-a41c-cf096e0b2508")

project(vk-weighted-blended LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_MODULE_STD 1)

# ----------------
# External dependencies.
# ----------------

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(vku CONFIG REQUIRED)

# ----------------
# Module configurations for external dependencies.
# ----------------

if (NOT TARGET glm_module)
    add_library(glm_module)
    target_sources(glm_module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS extlibs
        FILES extlibs/module-ports/glm.cppm
    )
    target_link_libraries(glm_module PUBLIC glm::glm)

    target_compile_definitions(glm_module PUBLIC
        GLM_GTC_INLINE_NAMESPACE
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_XYZW_ONLY
    )

    add_library(glm::module ALIAS glm_module)
endif()

# ----------------
# Project targets.
# ----------------

add_executable(vk-weighted-blended main.cpp)
target_sources(vk-weighted-blended PRIVATE
    FILE_SET CXX_MODULES
    FILES
        interface/mod.cppm
        interface/MainApp.cppm
        interface/vulkan/attachment_group/Composition.cppm
        interface/vulkan/attachment_group/Opaque.cppm
        interface/vulkan/attachment_group/WeightedBlended.cppm
        interface/vulkan/buffer/CubeInstances.cppm
        interface/vulkan/descriptor_set_layout/WeightedBlendedInput.cppm
        interface/vulkan/Frame.cppm
        interface/vulkan/Gpu.cppm
        interface/vulkan/mesh/Cube.cppm
        interface/vulkan/pipeline/CompositionRenderer.cppm
        interface/vulkan/pipeline/OpaqueRenderer.cppm
        interface/vulkan/pipeline/WeightedBlendedRenderer.cppm
        interface/vulkan/render_pass/WeightedBlended.cppm
        interface/vulkan/SharedData.cppm
)
target_link_libraries(vk-weighted-blended PRIVATE
    glfw
    glm::module
    vku::vku
)
target_compile_definitions(vk-weighted-blended PRIVATE
    GLFW_INCLUDE_NONE
    COMPILED_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)