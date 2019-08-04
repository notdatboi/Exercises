#include<BasicMesh.hpp>

void BasicMesh::createPipeline(const uint32_t pipelineIndex, const std::vector<std::string/*spk::ShaderInfo*/> shaderInfos, const vk::Extent2D extent, const vk::PipelineLayout layout, const vk::RenderPass renderPass, const uint32_t subpassIndex)
{
    if(pipelineIndex >= shaderSets.size())
    {
        throw std::invalid_argument("Pipeline index is out of range.\n");
    }
    loadShaders(pipelineIndex, shaderInfos);

    pipelines[pipelineIndex].addShaderStages(shaderSets[pipelineIndex].getShaderStages());

    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(stride);

    pipelines[pipelineIndex].addVertexInputState({bindingDesc}, vertexDescription.getAttributeDescriptions())
        .addInputAssemblyState(vk::PrimitiveTopology::ePatchList, false)
        .addTessellationState(3);

    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent({extent.width, extent.height});

    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(extent.width)
        .setHeight(extent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);

    pipelines[pipelineIndex].addViewportState({viewport}, {scissor})
        .addRasterizationState(false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false, 0, 0, 0, 1)
        .addDepthStencilState(true, true, vk::CompareOp::eLess, false, false, vk::StencilOpState(), vk::StencilOpState(), 0, 1);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.setBlendEnable(false)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

    pipelines[pipelineIndex].addColorBlendState(false, vk::LogicOp::eEquivalent, {colorBlendAttachmentState}, {1, 1, 1, 1})
        .setLayout(layout)
        .setRenderPass(renderPass)
        .setSubpassIndex(subpassIndex);

    pipelines[pipelineIndex].create();
}