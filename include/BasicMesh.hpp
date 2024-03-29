#ifndef BASIC_MESH_HPP
#define BASIC_MESH_HPP
#include<Mesh.hpp>

class BasicMesh : public Mesh
{
public:
    virtual void createPipeline(const uint32_t pipelineIndex, const std::vector<std::string/*spk::ShaderInfo*/> shaderInfos, const vk::Extent2D extent, const vk::PipelineLayout layout, const vk::RenderPass renderPass, const uint32_t subpassIndex);
private:
};

#endif