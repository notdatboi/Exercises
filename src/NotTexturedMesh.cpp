#include<NotTexturedMesh.hpp>

void NotTexturedMesh::createPipeline(const uint32_t pipelineIndex, const std::vector<spk::ShaderInfo> shaderInfos, const vk::Extent2D extent, const spk::AdditionalInfo& info)
{
    if(pipelineIndex >= shaderSets.size())
    {
        throw std::invalid_argument("Pipeline index is out of range.\n");
    }
    loadShaders(pipelineIndex, shaderInfos);

    spk::ShaderStages shaderStages;
    shaderStages.stages = shaderSets[pipelineIndex].getShaderStages();

    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(stride);

    spk::VertexInputState vertexInputState;
    vertexInputState.bindingDescriptions = {bindingDesc};
    vertexInputState.attributeDescriptions = vertexDescription.getAttributeDescriptions();

    spk::InputAssemblyState assemblyState;
    assemblyState.enablePrimitiveRestart = false;
    assemblyState.topology = vk::PrimitiveTopology::eTriangleList;

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

    spk::TessellationState tessellationState;

    spk::ViewportState viewportState;
    viewportState.scissor = scissor;
    viewportState.viewport = viewport;

    spk::RasterizationState rasterizationState;
    rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationState.enableDepthClamp = false;
    rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;

    spk::MultisampleState multisampleState;
    multisampleState.rasterizationSampleCount = vk::SampleCountFlagBits::e1;

    spk::DepthStencilState depthStencilState;
    depthStencilState.enableDepthTest = true;
    depthStencilState.depthCompareOp = vk::CompareOp::eLess;
    depthStencilState.writeTestResults = true;

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
    colorBlendAttachmentState.setBlendEnable(false)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
    spk::ColorBlendState colorBlendState;
    colorBlendState.attachmentStates = {colorBlendAttachmentState};

    spk::DynamicState dynamicState;

    pipelines[pipelineIndex].create(shaderStages, 
        vertexInputState,
        assemblyState,
        tessellationState,
        viewportState,
        rasterizationState,
        multisampleState,
        depthStencilState,
        colorBlendState,
        dynamicState,
        info);
}