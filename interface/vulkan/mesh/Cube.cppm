export module vk_weighted_blended:vulkan.mesh.Cube;

import std;
import glm;
export import vk_mem_alloc_hpp;
import vku;

namespace vk_weighted_blended::vulkan::mesh {
    export struct Cube {
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
        };

        static constexpr std::uint32_t drawCount = 36;

        vku::AllocatedBuffer vertexBuffer;

        explicit Cube(
            vma::Allocator allocator
        ) : vertexBuffer { vku::MappedBuffer { allocator, std::from_range, std::array<Vertex, drawCount> {
                // Back face
                Vertex { { -0.5f, -0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // Bottom-left
                Vertex { {  0.5f,  0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // top-right
                Vertex { {  0.5f, -0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // bottom-right
                Vertex { {  0.5f,  0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // top-right
                Vertex { { -0.5f, -0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // bottom-left
                Vertex { { -0.5f,  0.5f, -0.5f }, {  0.f,  0.f, -1.f } }, // top-left
                // Front face
                Vertex { { -0.5f, -0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // bottom-left
                Vertex { {  0.5f, -0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // bottom-right
                Vertex { {  0.5f,  0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // top-right
                Vertex { {  0.5f,  0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // top-right
                Vertex { { -0.5f,  0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // top-left
                Vertex { { -0.5f, -0.5f,  0.5f }, {  0.f,  0.f,  1.f } }, // bottom-left
                // Left face
                Vertex { { -0.5f,  0.5f,  0.5f }, { -1.f,  0.f,  0.f } }, // top-right
                Vertex { { -0.5f,  0.5f, -0.5f }, { -1.f,  0.f,  0.f } }, // top-left
                Vertex { { -0.5f, -0.5f, -0.5f }, { -1.f,  0.f,  0.f } }, // bottom-left
                Vertex { { -0.5f, -0.5f, -0.5f }, { -1.f,  0.f,  0.f } }, // bottom-left
                Vertex { { -0.5f, -0.5f,  0.5f }, { -1.f,  0.f,  0.f } }, // bottom-right
                Vertex { { -0.5f,  0.5f,  0.5f }, { -1.f,  0.f,  0.f } }, // top-right
                // Right face
                Vertex { {  0.5f,  0.5f,  0.5f }, {  1.f,  0.f,  0.f } }, // top-left
                Vertex { {  0.5f, -0.5f, -0.5f }, {  1.f,  0.f,  0.f } }, // bottom-right
                Vertex { {  0.5f,  0.5f, -0.5f }, {  1.f,  0.f,  0.f } }, // top-right
                Vertex { {  0.5f, -0.5f, -0.5f }, {  1.f,  0.f,  0.f } }, // bottom-right
                Vertex { {  0.5f,  0.5f,  0.5f }, {  1.f,  0.f,  0.f } }, // top-left
                Vertex { {  0.5f, -0.5f,  0.5f }, {  1.f,  0.f,  0.f } }, // bottom-left
                // Bottom face
                Vertex { { -0.5f, -0.5f, -0.5f }, {  0.f, -1.f,  0.f } }, // top-right
                Vertex { {  0.5f, -0.5f, -0.5f }, {  0.f, -1.f,  0.f } }, // top-left
                Vertex { {  0.5f, -0.5f,  0.5f }, {  0.f, -1.f,  0.f } }, // bottom-left
                Vertex { {  0.5f, -0.5f,  0.5f }, {  0.f, -1.f,  0.f } }, // bottom-left
                Vertex { { -0.5f, -0.5f,  0.5f }, {  0.f, -1.f,  0.f } }, // bottom-right
                Vertex { { -0.5f, -0.5f, -0.5f }, {  0.f, -1.f,  0.f } }, // top-right
                // Top face
                Vertex { { -0.5f,  0.5f, -0.5f }, {  0.f,  1.f,  0.f } }, // top-left
                Vertex { {  0.5f,  0.5f,  0.5f }, {  0.f,  1.f,  0.f } }, // bottom-right
                Vertex { {  0.5f,  0.5f, -0.5f }, {  0.f,  1.f,  0.f } }, // top-right
                Vertex { {  0.5f,  0.5f,  0.5f }, {  0.f,  1.f,  0.f } }, // bottom-right
                Vertex { { -0.5f,  0.5f, -0.5f }, {  0.f,  1.f,  0.f } }, // top-left
                Vertex { { -0.5f,  0.5f,  0.5f }, {  0.f,  1.f,  0.f } }, // bottom-left
            }, vk::BufferUsageFlagBits::eVertexBuffer }.unmap() } { }
    };
}