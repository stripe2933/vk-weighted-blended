export module vk_weighted_blended:vulkan.pipeline.WeightedBlendedRenderer;

#ifdef _MSC_VER
import std;
#endif
export import glm;
export import vulkan_hpp;
import vku;
import :shader.weighted_blended_frag;
import :shader.weighted_blended_vert;
export import :vulkan.rp.WeightedBlended;

namespace vk_weighted_blended::vulkan::inline pipeline {
    export struct WeightedBlendedRenderer {
        struct PushConstant {
            glm::mat4 projectionView;
            glm::vec3 viewPosition;
        };

        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        WeightedBlendedRenderer(
            const vk::raii::Device &device [[clang::lifetimebound]],
            const rp::WeightedBlended &renderPass [[clang::lifetimebound]]
        ) : pipelineLayout { device, vk::PipelineLayoutCreateInfo {
                {},
                {},
                vku::unsafeProxy({
                    vk::PushConstantRange { vk::ShaderStageFlagBits::eAllGraphics, 0, sizeof(PushConstant) },
                }),
            } },
            pipeline { device, nullptr, vku::getDefaultGraphicsPipelineCreateInfo(
                vku::createPipelineStages(
                    device,
                    vku::Shader { shader::weighted_blended_vert, vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { shader::weighted_blended_frag, vk::ShaderStageFlagBits::eFragment }).get(),
                *pipelineLayout, 2, true, vk::SampleCountFlagBits::e4)
                .setPVertexInputState(vku::unsafeAddress(vk::PipelineVertexInputStateCreateInfo {
                    {},
                    vku::unsafeProxy({
                        vk::VertexInputBindingDescription { 0, sizeof(glm::vec3) * 2, vk::VertexInputRate::eVertex },
                        vk::VertexInputBindingDescription { 1, sizeof(glm::vec3) + sizeof(glm::vec4), vk::VertexInputRate::eInstance },
                    }),
                    vku::unsafeProxy({
                        // inPosition
                        vk::VertexInputAttributeDescription { 0, 0, vk::Format::eR32G32B32Sfloat, 0 },
                        // inNormal
                        vk::VertexInputAttributeDescription { 1, 0, vk::Format::eR32G32B32Sfloat, sizeof(glm::vec3) },
                        // inInstancePosition
                        vk::VertexInputAttributeDescription { 2, 1, vk::Format::eR32G32B32Sfloat, 0 },
                        // inInstanceColor
                        vk::VertexInputAttributeDescription { 3, 1, vk::Format::eR32G32B32A32Sfloat, sizeof(glm::vec3) },
                    }),
                }))
                .setPRasterizationState(vku::unsafeAddress(vk::PipelineRasterizationStateCreateInfo {
                    {},
                    false, false,
                    vk::PolygonMode::eFill,
                    // Translucent objects' back faces shouldn't be culled.
                    vk::CullModeFlagBits::eNone, {},
                    false, {}, {}, {},
                    1.f,
                }))
                .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
                    {},
                    // Translucent objects shouldn't interfere with the pre-rendered depth buffer.
                    true, false, vk::CompareOp::eLess,
                }))
                .setPColorBlendState(vku::unsafeAddress(vk::PipelineColorBlendStateCreateInfo {
                    {},
                    false, {},
                    vku::unsafeProxy({
                        vk::PipelineColorBlendAttachmentState {
                            true,
                            vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                            vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                        },
                        vk::PipelineColorBlendAttachmentState {
                            true,
                            vk::BlendFactor::eZero, vk::BlendFactor::eOneMinusSrcColor, vk::BlendOp::eAdd,
                            vk::BlendFactor::eZero, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                            vk::ColorComponentFlagBits::eR,
                        },
                    }),
                    { 1.f, 1.f, 1.f, 1.f },
                }))
                .setRenderPass(*renderPass)
                .setSubpass(1)
            } { }
    };
}