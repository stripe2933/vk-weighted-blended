export module vk_weighted_blended:vulkan.ag.Composition;

import std;
import vku;
export import :vulkan.Gpu;

namespace vk_weighted_blended::vulkan::ag {
    export struct Composition final : vku::AttachmentGroup {
        Composition(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            std::span<const vk::Image> swapchainImages
        ) : AttachmentGroup { extent } {
            addSwapchainAttachment(device, swapchainImages, vk::Format::eB8G8R8A8Srgb);
        }
    };
}