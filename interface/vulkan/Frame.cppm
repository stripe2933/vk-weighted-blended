module;

#include <cassert>

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_weighted_blended:vulkan.Frame;

import std;
import vku;
export import :vulkan.SharedData;
import :vulkan.ag.Opaque;
import :vulkan.ag.WeightedBlended;

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

namespace vk_weighted_blended::vulkan {
    export class Frame {
    public:
        Frame(
            const Gpu &gpu [[clang::lifetimebound]],
            const SharedData &sharedData [[clang::lifetimebound]]
        ) : gpu { gpu },
            sharedData { sharedData } {
            std::tie(weightedBlendedInputDescriptorSet)
                = vku::allocateDescriptorSets(*gpu.device, *descriptorPool, std::tie(
                    sharedData.weightedBlendedInputDescriptorSetLayout));

            // Update per-frame descriptors.
            gpu.device.updateDescriptorSets({
                weightedBlendedInputDescriptorSet.getWriteOne<0>({ {}, *weightedBlendedAttachmentGroup.colorAttachments[0].resolveView, vk::ImageLayout::eShaderReadOnlyOptimal }),
                weightedBlendedInputDescriptorSet.getWriteOne<1>({ {}, *weightedBlendedAttachmentGroup.colorAttachments[1].resolveView, vk::ImageLayout::eShaderReadOnlyOptimal }),
            }, {});

            // Initialize attachment layouts.
            vku::executeSingleCommand(*gpu.device, *commandPool, gpu.queues.graphicsPresent, [this](vk::CommandBuffer cb) {
                recordAttachmentLayoutInitializationCommands(cb);
            });
            gpu.queues.graphicsPresent.waitIdle();
        }

        auto onLoop(
            float time
        ) -> void {
            // Wait for the previous frame to finish.
            const vk::Result frameFinishResult = gpu.device.waitForFences(*frameFinishFence, true, ~0U);
            assert(frameFinishResult == vk::Result::eSuccess && "Failed to wait for frame finish fence.");
            gpu.device.resetFences(*frameFinishFence);

            const glm::vec3 eye = 15.f * glm::vec3 { std::cos(0.1f * time), 1.f, std::sin(0.1f * time) };
            const glm::mat4 projectionView
                = glm::perspective(glm::radians(45.f), vku::aspect(sharedData.swapchainExtent), 0.1f, 100.f)
                * lookAt(eye, glm::vec3 { 0.f }, { 0.f, 1.f, 0.f });

            // Acquire swapchain image to render.
            const auto [swapchainImageAvailableResult, swapchainImageIndex] = (*gpu.device).acquireNextImageKHR(*sharedData.swapchain, ~0U, *swapchainImageAvailableSemaphore);
            assert(swapchainImageAvailableResult == vk::Result::eSuccess && "Failed to acquire next swapchain image.");

            commandPool.reset();
            commandBuffer.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

            // Set viewport and scissor, which are pipeline dynamic states.
            commandBuffer.setViewport(0, vku::toViewport(sharedData.swapchainExtent, true));
            commandBuffer.setScissor(0, vk::Rect2D { { 0, 0 }, sharedData.swapchainExtent });

            commandBuffer.beginRenderPass(vk::RenderPassBeginInfo {
                *sharedData.weightedBlendedRenderPass,
                *framebuffers[swapchainImageIndex],
                vk::Rect2D { { 0, 0 }, sharedData.swapchainExtent },
                vku::unsafeProxy<vk::ClearValue>({
                    vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                    vk::ClearColorValue{},
                    vk::ClearDepthStencilValue { 1.f, 0 },
                    vk::ClearColorValue { 0.f, 0.f, 0.f, 0.f },
                    vk::ClearColorValue{},
                    vk::ClearColorValue { 1.f, 0.f, 0.f, 0.f },
                    vk::ClearColorValue{},
                }),
            }, vk::SubpassContents::eInline);

            // 1st subpass: opaque rendering.
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.opaqueRenderer.pipeline);
            commandBuffer.bindVertexBuffers(0, { sharedData.cubeMesh.vertexBuffer.buffer, sharedData.cubeInstanceBuffer.buffer }, { 0, 0 });
            commandBuffer.pushConstants<WeightedBlendedRenderer::PushConstant>(*sharedData.opaqueRenderer.pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, WeightedBlendedRenderer::PushConstant {
                .projectionView = projectionView,
                .viewPosition = eye,
            });
            commandBuffer.draw(sharedData.cubeMesh.drawCount, 25, 0, 0);

            commandBuffer.nextSubpass(vk::SubpassContents::eInline);

            // 2st subpass: color/revealage accumulation.
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.weightedBlendedRenderer.pipeline);
            // Already bound by the previous subpass.
            // commandBuffer.bindVertexBuffers(0, { sharedData.cubeMesh.vertexBuffer.buffer, sharedData.cubeInstanceBuffer.buffer }, { 0, 0 });
            // commandBuffer.pushConstants<WeightedBlendedRenderer::PushConstant>(*sharedData.weightedBlendedRenderer.pipelineLayout, vk::ShaderStageFlagBits::eAllGraphics, 0, WeightedBlendedRenderer::PushConstant {
            //     .projectionView = projectionView,
            //     .viewPosition = eye,
            // });
            commandBuffer.draw(sharedData.cubeMesh.drawCount, 100, 0, 25);

            commandBuffer.nextSubpass(vk::SubpassContents::eInline);

            // 3rd subpass: composition.
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *sharedData.compositionRenderer.pipeline);
            commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *sharedData.compositionRenderer.pipelineLayout, 0, weightedBlendedInputDescriptorSet, {});
            commandBuffer.draw(3, 1, 0, 0);

            commandBuffer.endRenderPass();

            commandBuffer.end();

            // Submit commandBuffer to queue.
            gpu.queues.graphicsPresent.submit(vk::SubmitInfo {
                *swapchainImageAvailableSemaphore,
                vku::unsafeProxy({ vk::Flags { vk::PipelineStageFlagBits::eColorAttachmentOutput } }),
                commandBuffer,
                *drawFinishSemaphore,
            }, *frameFinishFence);

            // Present swapchain image.
            const vk::Result swapchainImagePresentResult = gpu.queues.graphicsPresent.presentKHR(vk::PresentInfoKHR {
                *drawFinishSemaphore,
                *sharedData.swapchain,
                swapchainImageIndex,
            });
            assert(swapchainImagePresentResult == vk::Result::eSuccess && "Failed to present swapchain image.");
        }

    private:
        const Gpu &gpu;
        const SharedData &sharedData;

        // --------------------
        // GPU resources.
        // --------------------

        vku::AllocatedImage opaqueColorImage = createOpaqueColorImage();
        vku::AllocatedImage depthImage = createDepthImage();

        // --------------------
        // Frame-exclusive attachment groups.
        // --------------------

        std::vector<ag::Opaque> opaqueAttachmentGroups = createOpaqueAttachmentGroups();
        ag::WeightedBlended weightedBlendedAttachmentGroup { gpu, sharedData.swapchainExtent, depthImage };

        // --------------------
        // Framebuffers.
        // --------------------

        std::vector<vk::raii::Framebuffer> framebuffers = createFramebuffers();

        // --------------------
        // Descriptor pools and sets.
        // --------------------

        vk::raii::DescriptorPool descriptorPool = createDescriptorPool();
        vku::DescriptorSet<dsl::WeightedBlendedInput> weightedBlendedInputDescriptorSet;

        // --------------------
        // Command pools and buffers.
        // --------------------

        vk::raii::CommandPool commandPool = createCommandPool();
        vk::CommandBuffer commandBuffer = (*gpu.device).allocateCommandBuffers(vk::CommandBufferAllocateInfo {
            *commandPool,
            vk::CommandBufferLevel::ePrimary,
            1,
        })[0];

        // --------------------
        // Synchronization stuffs.
        // --------------------

        vk::raii::Semaphore swapchainImageAvailableSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Semaphore drawFinishSemaphore { gpu.device, vk::SemaphoreCreateInfo{} };
        vk::raii::Fence frameFinishFence { gpu.device, vk::FenceCreateInfo { vk::FenceCreateFlagBits::eSignaled } };

        [[nodiscard]] auto createOpaqueColorImage() const -> vku::AllocatedImage {
            return { gpu.allocator, vk::ImageCreateInfo {
                {},
                vk::ImageType::e2D,
                vk::Format::eB8G8R8A8Srgb,
                vk::Extent3D { sharedData.swapchainExtent, 1 },
                1, 1,
                vk::SampleCountFlagBits::e4,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
            }, vku::allocation::deviceLocalTransient };
        }

        [[nodiscard]] auto createDepthImage() const -> vku::AllocatedImage {
            return {
                gpu.allocator,
                vk::ImageCreateInfo {
                    {},
                    vk::ImageType::e2D,
                    vk::Format::eD32Sfloat,
                    vk::Extent3D { sharedData.swapchainExtent, 1 },
                    1, 1,
                    vk::SampleCountFlagBits::e4,
                    vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransientAttachment,
                },
#if __APPLE__
                // MoltenVK bug. Described in https://github.com/stripe2933/vk-deferred/blob/75bf7536f4c9c6af76fe9875853f9e785ca1dfb2/interface/vulkan/attachment_group/GBuffer.cppm#L28.
                vku::allocation::deviceLocal,
#else
                vku::allocation::deviceLocalTransient,
#endif
            };
        }

        [[nodiscard]] auto createOpaqueAttachmentGroups() const -> std::vector<ag::Opaque> {
            return sharedData.swapchainImages
                | std::views::transform([this](vk::Image swapchainImage) {
                    return ag::Opaque {
                        gpu,
                        sharedData.swapchainExtent,
                        opaqueColorImage,
                        vku::Image { swapchainImage, vk::Extent3D { sharedData.swapchainExtent, 1 }, vk::Format::eB8G8R8A8Srgb, 1, 1 },
                        depthImage,
                    };
                })
                | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto createDescriptorPool() const -> vk::raii::DescriptorPool {
            return { gpu.device, sharedData.weightedBlendedInputDescriptorSetLayout.getPoolSize().getDescriptorPoolCreateInfo() };
        }

        [[nodiscard]] auto createFramebuffers() const -> std::vector<vk::raii::Framebuffer> {
            return opaqueAttachmentGroups
                | std::views::transform([&](const auto &opaqueAttachmentGroup) {
                    return vk::raii::Framebuffer { gpu.device, vk::FramebufferCreateInfo {
                        {},
                        *sharedData.weightedBlendedRenderPass,
                        vku::unsafeProxy({
                            *opaqueAttachmentGroup.colorAttachments[0].view,
                            *opaqueAttachmentGroup.colorAttachments[0].resolveView,
                            *opaqueAttachmentGroup.depthStencilAttachment->view,
                            *weightedBlendedAttachmentGroup.colorAttachments[0].view,
                            *weightedBlendedAttachmentGroup.colorAttachments[0].resolveView,
                            *weightedBlendedAttachmentGroup.colorAttachments[1].view,
                            *weightedBlendedAttachmentGroup.colorAttachments[1].resolveView,
                        }),
                        sharedData.swapchainExtent.width, sharedData.swapchainExtent.height, 1,
                    } };
                })
                | std::ranges::to<std::vector>();
        }

        [[nodiscard]] auto createCommandPool() const -> vk::raii::CommandPool {
            return { gpu.device, vk::CommandPoolCreateInfo {
                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                gpu.queueFamilies.graphicsPresent,
            } };
        }

        auto recordAttachmentLayoutInitializationCommands(
            vk::CommandBuffer cb
        ) const -> void {
            // Initialize the image layouts as desired end layout of the frame, for avoid the undefined layout for
            // initialLayout in render passes.
            cb.pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eBottomOfPipe,
                {}, {}, {},
                {
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eColorAttachmentOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        opaqueColorImage, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eDepthStencilAttachmentOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        depthImage, vku::fullSubresourceRange(vk::ImageAspectFlagBits::eDepth),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eColorAttachmentOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        weightedBlendedAttachmentGroup.colorAttachments[0].image, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        weightedBlendedAttachmentGroup.colorAttachments[0].resolveImage, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eColorAttachmentOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        weightedBlendedAttachmentGroup.colorAttachments[1].image, vku::fullSubresourceRange(),
                    },
                    vk::ImageMemoryBarrier {
                        {}, {},
                        {}, vk::ImageLayout::eShaderReadOnlyOptimal,
                        vk::QueueFamilyIgnored, vk::QueueFamilyIgnored,
                        weightedBlendedAttachmentGroup.colorAttachments[1].resolveImage, vku::fullSubresourceRange(),
                    },
                });
        }
    };
}