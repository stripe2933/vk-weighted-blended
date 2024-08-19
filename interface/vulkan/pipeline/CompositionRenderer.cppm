export module vk_weighted_blended:vulkan.pipeline.CompositionRenderer;

#ifdef _MSC_VER
import std;
#endif
export import vulkan_hpp;
import vku;
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
#ifdef _MSC_VER
                // TODO: due to the MSVC C++20 module bug, vku::createPipelineStages not works well. Use it instead when fixed.
                vku::unsafeProxy({
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eVertex,
                        *vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/composition.vert.spv", vk::ShaderStageFlagBits::eVertex }.code),
                        } },
                        "main",
                    },
                    vk::PipelineShaderStageCreateInfo {
                        {},
                        vk::ShaderStageFlagBits::eFragment,
                        *vk::raii::ShaderModule { device, vk::ShaderModuleCreateInfo {
                            {},
                            vku::unsafeProxy(vku::Shader { COMPILED_SHADER_DIR "/composition.frag.spv", vk::ShaderStageFlagBits::eFragment }.code),
                        } },
                        "main",
                    },
                }),
#else
                vku::createPipelineStages(
                    device,
                    vku::Shader { COMPILED_SHADER_DIR "/composition.vert.spv", vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { COMPILED_SHADER_DIR "/composition.frag.spv", vk::ShaderStageFlagBits::eFragment }).get(),
#endif
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