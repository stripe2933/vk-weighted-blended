export module vk_weighted_blended:vulkan.dsl.WeightedBlendedInput;

#ifdef _MSC_VER
import std;
#endif
import vku;

namespace vk_weighted_blended::vulkan::dsl {
    export struct WeightedBlendedInput final : vku::DescriptorSetLayout<vk::DescriptorType::eInputAttachment, vk::DescriptorType::eInputAttachment> {
        explicit WeightedBlendedInput(
            const vk::raii::Device &device [[clang::lifetimebound]]
        ) : DescriptorSetLayout {
                device,
                vk::DescriptorSetLayoutCreateInfo {
                    {},
                    vku::unsafeProxy(getBindings(
                        { 1, vk::ShaderStageFlagBits::eFragment }, /* Accumulation. */
                        { 1, vk::ShaderStageFlagBits::eFragment } /* Revealage. */)),
                },
            } { }
    };
}