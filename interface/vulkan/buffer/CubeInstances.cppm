module;

#include <cassert>

export module vk_weighted_blended:vulkan.buffer.CubeInstances;

import std;
import glm;
import vku;
export import vk_mem_alloc_hpp;

namespace vk_weighted_blended::vulkan::buffer {
    class CubeInstances : public vku::AllocatedBuffer {
    public:
        struct Instance {
            glm::vec3 position;
            glm::vec4 color;
        };

        static constexpr std::size_t instanceCount = 125;

        explicit CubeInstances(
            vma::Allocator allocator
        ) : AllocatedBuffer { vku::MappedBuffer {
                allocator, std::from_range, getInstances(25), vk::BufferUsageFlagBits::eVertexBuffer
            }.unmap() } { }

    private:
        [[nodiscard]] static auto getInstances(std::size_t opaqueInstance) -> std::vector<Instance> {
            assert(opaqueInstance < instanceCount);

            std::vector<glm::vec3> positions;
            for (auto x : { -5.f, -2.5f, 0.f, 2.5f, 5.f }) {
                for (auto y : { -5.f, -2.5f, 0.f, 2.5f, 5.f }) {
                    for (auto z : { -5.f, -2.5f, 0.f, 2.5f, 5.f }) {
                        positions.emplace_back(x, y, z);
                    }
                }
            }

            std::random_device rd;
            std::mt19937 gen { rd() };
            std::ranges::shuffle(positions, gen);

            // 25 visually distinct colors.
            std::vector<glm::vec3> distinctColors {
                { 0.753f, 0.753f, 0.753f },
                { 0.184f, 0.310f, 0.310f },
                { 0.333f, 0.420f, 0.184f },
                { 0.647f, 0.165f, 0.165f },
                { 0.282f, 0.239f, 0.545f },
                { 0.235f, 0.702f, 0.443f },
                { 0.000f, 0.000f, 0.502f },
                { 0.604f, 0.804f, 0.196f },
                { 0.545f, 0.000f, 0.545f },
                { 0.282f, 0.820f, 0.800f },
                { 1.000f, 0.000f, 0.000f },
                { 1.000f, 0.549f, 0.000f },
                { 1.000f, 1.000f, 0.000f },
                { 0.498f, 1.000f, 0.000f },
                { 0.541f, 0.169f, 0.886f },
                { 0.000f, 1.000f, 0.498f },
                { 0.000f, 0.749f, 1.000f },
                { 0.000f, 0.000f, 1.000f },
                { 1.000f, 0.000f, 1.000f },
                { 0.118f, 0.565f, 1.000f },
                { 0.859f, 0.439f, 0.576f },
                { 0.941f, 0.902f, 0.549f },
                { 1.000f, 0.078f, 0.576f },
                { 1.000f, 0.627f, 0.478f },
                { 0.933f, 0.510f, 0.933f },
            };

            // First opaqueInstances are opaque, the rest are translucent.
            std::vector<Instance> result;
            std::size_t i = 0;
            for (; i < opaqueInstance; ++i) {
                result.emplace_back(positions[i], glm::vec4 { distinctColors[i], 1.f });
            }
            for (; i < instanceCount; ++i) {
                result.emplace_back(positions[i], glm::vec4 { distinctColors[i % 25], 0.5f });
            }
            return result;
        }
    };
}