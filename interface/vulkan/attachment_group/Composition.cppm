export module vk_weighted_blended:vulkan.ag.Composition;

#ifdef _MSC_VER
import std;
#endif
import vku;
export import :vulkan.Gpu;

namespace vk_weighted_blended::vulkan::ag {
    export struct Composition final : vku::AttachmentGroup {
        Composition(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            const vku::Image &swapchainImage
        ) : AttachmentGroup { extent } {
            addColorAttachment(device, swapchainImage);
        }
    };
}