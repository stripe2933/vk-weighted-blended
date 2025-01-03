export module vk_weighted_blended:vulkan.pipeline.CompositionRenderer;

#ifdef _MSC_VER
import std;
#endif
export import vulkan_hpp;
import vku;
import :shader.composition_frag;
import :shader.composition_vert;
export import :vulkan.dsl.WeightedBlendedInput;
export import :vulkan.rp.WeightedBlended;

namespace vk_weighted_blended::vulkan::inline pipeline {
    export struct CompositionRenderer {
        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        CompositionRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const dsl::WeightedBlendedInput &descriptorSetLayout [[clang::lifetimebound]],
            const rp::WeightedBlended &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                vku::unsafeProxy(*descriptorSetLayout),
            } },
            pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
                vku::createPipelineStages(
                    device,
                    vku::Shader { shader::composition_vert, vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { shader::composition_frag, vk::ShaderStageFlagBits::eFragment }).get(),
                *pipelineLayout, 1)
                .setPColorBlendState(vku::unsafeAddress(vk::PipelineColorBlendStateCreateInfo {
                    {},
                    false, {},
                    vku::unsafeProxy({
                        vk::PipelineColorBlendAttachmentState {
                            true,
                            vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                            vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                        },
                    }),
                    { 1.f, 1.f, 1.f, 1.f },
                }))
                .setRenderPass(*renderPass)
                .setSubpass(2)
            } { }
    };
}