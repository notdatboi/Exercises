#include"BasicMesh.hpp"

void BasicMesh::createPipeline(const uint32_t pipelineIndex, const std::vector<spk::ShaderInfo> shaderInfos, const vk::Extent2D extent, const spk::AdditionalInfo& info)
{
    if(pipelines.size() <= pipelineIndex)
    {
        for(auto index = pipelines.size(); index <= pipelineIndex; ++index)
        {
            pipelines.push_back(spk::Pipeline());
        }
    }
    loadShaders(pipelineIndex, shaderInfos);

    spk::ShaderStages shaderStages;
    shaderStages.stages = shaderSets[pipelineIndex].getShaderStages();

    vk::VertexInputBindingDescription bindingDesc;
    bindingDesc.setBinding(0)
        .setInputRate(vk::VertexInputRate::eVertex)
        .setStride(sizeof(Vertex));

    vk::VertexInputAttributeDescription positionAttribute, normalAttribute, uvAttribute;
    positionAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setLocation(0)
        .setOffset(offsetof(Vertex, position));
    normalAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32B32Sfloat)
        .setLocation(1)
        .setOffset(offsetof(Vertex, normal));
    uvAttribute.setBinding(0)
        .setFormat(vk::Format::eR32G32Sfloat)
        .setLocation(2)
        .setOffset(offsetof(Vertex, uv));

    spk::VertexInputState vertexInputState;
    vertexInputState.bindingDescriptions = {bindingDesc};
    vertexInputState.attributeDescriptions = {positionAttribute, normalAttribute, uvAttribute};

    spk::InputAssemblyState assemblyState;
    assemblyState.enablePrimitiveRestart = false;
    assemblyState.topology = vk::PrimitiveTopology::ePatchList;

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
    tessellationState.patchControlPointCount = 4;

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