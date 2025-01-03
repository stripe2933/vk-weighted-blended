export module vk_weighted_blended:vulkan.pipeline.OpaqueRenderer;

#ifdef _MSC_VER
import std;
#endif
export import glm;
export import vulkan_hpp;
import vku;
import :shader.opaque_frag;
import :shader.opaque_vert;
export import :vulkan.rp.WeightedBlended;

namespace vk_weighted_blended::vulkan::inline pipeline {
    export struct OpaqueRenderer {
        struct PushConstant {
            glm::mat4 projectionView;
            glm::vec3 viewPosition;
        };

        vk::raii::PipelineLayout pipelineLayout;
        vk::raii::Pipeline pipeline;

        OpaqueRenderer(
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
                    vku::Shader { shader::opaque_vert, vk::ShaderStageFlagBits::eVertex },
                    vku::Shader { shader::opaque_frag, vk::ShaderStageFlagBits::eFragment }).get(),
                *pipelineLayout, 1, true, vk::SampleCountFlagBits::e4)
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
                .setPDepthStencilState(vku::unsafeAddress(vk::PipelineDepthStencilStateCreateInfo {
                    {},
                    true, true, vk::CompareOp::eLess,
                }))
                .setRenderPass(*renderPass)
                .setSubpass(0)
            } { }
    };
}