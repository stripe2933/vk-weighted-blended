module;

#include <vulkan/vulkan_hpp_macros.hpp>

export module vk_weighted_blended:vulkan.Gpu;

import std;
import vku;
export import vulkan_hpp;

namespace vk_weighted_blended::vulkan {
    export class Gpu {
    public:
        struct QueueFamilies {
            std::uint32_t graphicsPresent;

            QueueFamilies(
                const vk::raii::PhysicalDevice &physicalDevice [[clang::lifetimebound]],
                vk::SurfaceKHR surface
            ) {
                for (std::uint32_t i = 0; vk::QueueFamilyProperties properties : physicalDevice.getQueueFamilyProperties()) {
                    if (properties.queueFlags & vk::QueueFlagBits::eGraphics && physicalDevice.getSurfaceSupportKHR(i, surface)) {
                        graphicsPresent = i;
                        return;
                    }

                    ++i;
                }

                throw std::runtime_error { "No suitable queue family found." };
            }
        };

        struct Queues {
            vk::Queue graphicsPresent;

            Queues(
                const vk::raii::Device &device,
                const QueueFamilies &queueFamilies
            ) noexcept : graphicsPresent { *device.getQueue(queueFamilies.graphicsPresent, 0) } { }
        };

        vk::raii::PhysicalDevice physicalDevice;
        QueueFamilies queueFamilies;
        vk::raii::Device device = createDevice();
        Queues queues { device, queueFamilies };
        vma::Allocator allocator;

        Gpu(
            const vk::raii::Instance &instance [[clang::lifetimebound]],
            vk::SurfaceKHR surface
        ) : physicalDevice { selectPhysicalDevice(instance, surface) },
            queueFamilies { physicalDevice, surface },
            allocator { createAllocator(instance) } { }

        ~Gpu() {
            allocator.destroy();
        }

    private:
        [[nodiscard]] auto selectPhysicalDevice(
            const vk::raii::Instance &instance,
            vk::SurfaceKHR surface
        ) const -> vk::raii::PhysicalDevice {
            auto adequatePhysicalDevices
                = instance.enumeratePhysicalDevices()
                | std::views::filter([&](const vk::raii::PhysicalDevice &physicalDevice) {
                    try {
                        std::ignore = QueueFamilies { physicalDevice, surface };
                        return true;
                    }
                    catch (const std::runtime_error&) {
                        return false;
                    }
                });
            if (adequatePhysicalDevices.empty()) {
                throw std::runtime_error { "No suitable GPU for the application" };
            }

            return *std::ranges::max_element(adequatePhysicalDevices, {}, [&](const vk::raii::PhysicalDevice &physicalDevice) {
                std::uint32_t score = 0;

                const vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
                    score += 1000;
                }

                score += properties.limits.maxImageDimension2D;

                return score;
            });
        }

        [[nodiscard]] auto createDevice() const -> vk::raii::Device {
            vk::raii::Device device { physicalDevice, vk::DeviceCreateInfo {
                {},
                vku::unsafeProxy({
                    vk::DeviceQueueCreateInfo {
                        {},
                        queueFamilies.graphicsPresent,
                        vku::unsafeProxy({ 1.f }),
                    },
                }),
                {},
                vku::unsafeProxy({
                    vk::KHRSwapchainExtensionName,
#if __APPLE__
                    vk::KHRPortabilitySubsetExtensionName,
#endif
                }),
                vku::unsafeAddress(vk::PhysicalDeviceFeatures{}
                    .setIndependentBlend(true)),
            } };

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*device);
#endif
            return device;
        }

        [[nodiscard]] auto createAllocator(const vk::raii::Instance &instance) const -> vma::Allocator {
            return vma::createAllocator(vma::AllocatorCreateInfo {
                {},
                *physicalDevice, *device,
                {}, {}, {}, {},
#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
                vku::unsafeAddress(vma::VulkanFunctions{
                    instance.getDispatcher()->vkGetInstanceProcAddr,
                    device.getDispatcher()->vkGetDeviceProcAddr,
                }),
#else
                {},
#endif
                *instance, vk::makeApiVersion(0, 1, 2, 0),
            });
        }
    };
}