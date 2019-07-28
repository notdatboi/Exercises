#ifndef NOT_TEXTURED_MESH_HPP
#define NOT_TEXTURED_MESH_HPP

#include"Mesh.hpp"

class NotTexturedMesh : public Mesh
{
public:
    virtual void createPipeline(const uint32_t pipelineIndex, const std::vector<spk::ShaderInfo> shaderInfos, const vk::Extent2D extent, const spk::AdditionalInfo& info);
};

#endif