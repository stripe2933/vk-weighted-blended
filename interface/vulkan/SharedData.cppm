module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_weighted_blended:vulkan.SharedData;

import std;
import vku;
import :vulkan.buffer.CubeInstances;
export import :vulkan.Gpu;
import :vulkan.mesh.Cube;
export import :vulkan.pipeline.OpaqueRenderer;
export import :vulkan.pipeline.CompositionRenderer;
export import :vulkan.pipeline.WeightedBlendedRenderer;
import :vulkan.ag.Composition;

namespace vk_weighted_blended::vulkan {
    export class SharedData {
    public:
        // --------------------
        // Swapchain resources.
        // --------------------

        vk::Extent2D swapchainExtent; // Initialized by createSwapchain(const Gpu&, vk::SurfaceKHR).
        vk::raii::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchainImages;

        // --------------------
        // Render passes.
        // --------------------

        rp::WeightedBlended weightedBlendedRenderPass;

        // --------------------
        // Descriptor set layouts.
        // --------------------

        dsl::WeightedBlendedInput weightedBlendedInputDescriptorSetLayout;

        // --------------------
        // Pipelines.
        // --------------------

        OpaqueRenderer opaqueRenderer;
        CompositionRenderer compositionRenderer;
        WeightedBlendedRenderer weightedBlendedRenderer;

        // --------------------
        // GPU resources.
        // --------------------

        mesh::Cube cubeMesh;
        buffer::CubeInstances cubeInstanceBuffer;

        // --------------------
        // Attachment groups.
        // --------------------

        std::vector<ag::Composition> swapchainAttachmentGroups;

        SharedData(
            const Gpu &gpu [[clang::lifetimebound]],
            vk::SurfaceKHR surface
        ) : swapchain { createSwapchain(gpu, surface) },
            swapchainImages { (*gpu.device).getSwapchainImagesKHR(*swapchain) },
            weightedBlendedRenderPass { gpu.device },
            weightedBlendedInputDescriptorSetLayout { gpu.device },
            opaqueRenderer { gpu.device, weightedBlendedRenderPass },
            compositionRenderer { gpu.device, weightedBlendedInputDescriptorSetLayout, weightedBlendedRenderPass },
            weightedBlendedRenderer { gpu.device, weightedBlendedRenderPass },
            cubeMesh { gpu.allocator },
            cubeInstanceBuffer { gpu.allocator },
            swapchainAttachmentGroups { createSwapchainAttachmentGroups(gpu.device) }{
            const vk::raii::CommandPool commandPool { gpu.device, vk::CommandPoolCreateInfo {
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                gpu.queueFamilies.graphicsPresent,
            } };

            vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queues.graphicsPresent, [this](vk::CommandBuffer cb) {
                recordAttachmentLayoutInitializationCommands(cb);
            });
            gpu.queues.graphicsPresent.waitIdle();
        }

    private:
        [[nodiscard]] auto createSwapchain(const Gpu &gpu, vk::SurfaceKHR surface) -> vk::raii::SwapchainKHR {
            const vk::SurfaceCapabilitiesKHR surfaceCapabilities = gpu.physicalDevice.getSurfaceCapabilitiesKHR(surface);
            swapchainExtent = surfaceCapabilities.currentExtent;
            return { gpu.device, vk::SwapchainCreateInfoKHR {
                {},
                surface,
                std::min(surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount),
                vk::Format::eB8G8R8A8Srgb,
                vk::ColorSpaceKHR::eSrgbNonlinear,
                swapchainExtent,
                1,
                vk::ImageUsageFlagBits::eColorAttachment,
                vk::SharingMode::eExclusive, {},
                surfaceCapabilities.currentTransform,
                vk::CompositeAlphaFlagBitsKHR::eOpaque,
                vk::PresentModeKHR::eFifo,
            } };
        }

        [[nodiscard]] auto createSwapchainAttachmentGroups(
            const vk::raii::Device &device
        ) const -> std::vector<ag::Composition> {
            return swapchainImages
                | std::views::transform([&](vk::Image image) {
                    return ag::Composition {
                        device,
                        swapchainExtent,
                        vku::Image { image, vk::Extent3D { swapchainExtent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 },
                    };
                })
                | std::ranges::to<std::vector>();
        }

        auto recordAttachmentLayoutInitializationCommands(
            vk::CommandBuffer cb
        ) const -> void {
            const std::vector barriers
                = swapchainImages
                | std::views::transform([](vk::Image image) {
                    return vk::ImageMemoryBarrier{
                        {}, {},
                        {}, vk::ImageLayout::ePresentSrcKHR,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        image, vku::fullSubresourceRange(),
                    };
                })
                | std::ranges::to<std::vector>();

            // Initialize the image layouts as desired end layout of the frame, for avoid the undefined layout for
            // initialLayout in render passes.
            cb.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                {},
                // TODO: current MSVC C++20 module does not work well with vk::ArrayProxy<T>. Use it instead (and also inline) when fixed.
                {}, {}, {}, {}, barriers.size(), barriers.data());
        }
    };
}