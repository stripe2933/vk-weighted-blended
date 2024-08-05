export module vk_weighted_blended:vulkan.ag.Opaque;

#ifdef _MSC_VER
import std;
#endif
import vku;
export import :vulkan.Gpu;

namespace vk_weighted_blended::vulkan::ag {
    export struct Opaque final : vku::MsaaAttachmentGroup {
        Opaque(
            const Gpu &gpu [[clang::lifetimebound]],
            const vk::Extent2D &extent,
            const vku::Image &colorImage [[clang::lifetimebound]],
            const vku::Image &swapchainImage,
            const vku::Image &depthImage [[clang::lifetimebound]]
        ) : MsaaAttachmentGroup { extent, vk::SampleCountFlagBits::e4 } {
            addColorAttachment(gpu.device, colorImage, swapchainImage);
            setDepthStencilAttachment(gpu.device, depthImage);
        }
    };
}