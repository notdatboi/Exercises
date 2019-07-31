#include<Mesh.hpp>

void Mesh::create(const aiMesh& mesh, 
    const std::vector<vk::DescriptorSet>& descriptorSets,
    const uint32_t pipelineCount)
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    this->descriptorSets = descriptorSets;
    pipelines.resize(pipelineCount);
    shaderSets.resize(pipelineCount);

    vk::FenceCreateInfo fenceInfo;
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");
    if(logicalDevice.createFence(&fenceInfo, nullptr, bufferUpdateFences + 1) != vk::Result::eSuccess) throw std::runtime_error("Failed to create fence!\n");

    load(mesh);
}

void Mesh::load(const aiMesh& mesh)
{
    const auto& pool = spk::system::Executives::getInstance()->getPool();
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();

    vertexCount = mesh.mNumVertices;
    indexCount = 0;

    const auto faceCount = mesh.mNumFaces;
    for(auto currentFaceId = 0; currentFaceId < faceCount; ++currentFaceId)
    {
        const auto& face = *(mesh.mFaces + currentFaceId);
        indexCount += face.mNumIndices;
    }

    vk::CommandBufferAllocateInfo CBAllocInfo;
    CBAllocInfo.setCommandBufferCount(2)
        .setCommandPool(pool)
        .setLevel(vk::CommandBufferLevel::ePrimary);
    if(logicalDevice.allocateCommandBuffers(&CBAllocInfo, bufferUpdateCBs) != vk::Result::eSuccess) throw std::runtime_error("Failed to allocate command buffer!\n");

    size_t vertexSize = 0;

    int textureCoordsIndex = -1;
    for(auto index = 0; index < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++index)
    {
        if(mesh.HasTextureCoords(index))
        {
            textureCoordsIndex = index;
            break;
        }
    }
    if(mesh.HasPositions())
    {
        if(mesh.HasNormals())
        {
            if(textureCoordsIndex != -1)
            {
                if(mesh.HasTangentsAndBitangents())
                {
                    vertexDescription = BasicVertexDescriptions::VertexPNUTBDescription;
                    vertexSize = sizeof(VertexPNUTB);
                }
                else
                {
                    vertexDescription = BasicVertexDescriptions::VertexPNUDescription;
                    vertexSize = sizeof(VertexPNU);
                }
            }
            else
            {
                vertexDescription = BasicVertexDescriptions::VertexPNDescription;
                vertexSize = sizeof(VertexPN);
            }
        }
        else
        {
            vertexDescription = BasicVertexDescriptions::VertexPDescription;
            vertexSize = sizeof(VertexP);
        }
    }
    else throw std::invalid_argument("Invalid mesh.\n");

    stride = vertexSize;

    spk::Buffer vertexBufferOnCPU, indexBufferOnCPU;
    vertexBufferOnCPU.create(vertexCount * vertexSize, vk::BufferUsageFlagBits::eTransferSrc, false, false);
    indexBufferOnCPU.create(indexCount * sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferSrc, false, false);
    vertexBufferOnCPU.bindMemory();
    indexBufferOnCPU.bindMemory();
    void* vertexBufferDataPtr = vertexBufferOnCPU.getCPUAccessibleDataPtr();

    for(int index = 0; index < vertexCount; ++index)
    {
        if(mesh.HasPositions())
        {
            if(mesh.HasNormals())
            {
                if(textureCoordsIndex != -1)
                {
                    if(mesh.HasTangentsAndBitangents())
                    {
                        VertexPNUTB vertex;
                        vertex.position = glm::vec3((mesh.mVertices + index)->x, (mesh.mVertices + index)->y, (mesh.mVertices + index)->z);
                        vertex.normal = glm::vec3((mesh.mNormals + index)->x, (mesh.mNormals + index)->y, (mesh.mNormals + index)->z);
                        vertex.uv = glm::vec2((mesh.mTextureCoords[textureCoordsIndex] + index)->x, (mesh.mTextureCoords[textureCoordsIndex] + index)->y);
                        vertex.tangent = glm::vec3((mesh.mTangents + index)->x, (mesh.mTangents + index)->y, (mesh.mTangents + index)->z);
                        vertex.bitangent = glm::vec3((mesh.mBitangents + index)->x, (mesh.mBitangents + index)->y, (mesh.mBitangents + index)->z);
                        memcpy(vertexBufferDataPtr + sizeof(VertexPNUTB) * index, &vertex, sizeof(VertexPNUTB));
                    }
                    else
                    {
                        VertexPNU vertex;
                        vertex.position = glm::vec3((mesh.mVertices + index)->x, (mesh.mVertices + index)->y, (mesh.mVertices + index)->z);
                        vertex.normal = glm::vec3((mesh.mNormals + index)->x, (mesh.mNormals + index)->y, (mesh.mNormals + index)->z);
                        vertex.uv = glm::vec2((mesh.mTextureCoords[textureCoordsIndex] + index)->x, (mesh.mTextureCoords[textureCoordsIndex] + index)->y);
                        memcpy(vertexBufferDataPtr + sizeof(VertexPNU) * index, &vertex, sizeof(VertexPNU));
                    }
                }
                else
                {
                    VertexPN vertex;
                    vertex.position = glm::vec3((mesh.mVertices + index)->x, (mesh.mVertices + index)->y, (mesh.mVertices + index)->z);
                    vertex.normal = glm::vec3((mesh.mNormals + index)->x, (mesh.mNormals + index)->y, (mesh.mNormals + index)->z);
                    memcpy(vertexBufferDataPtr + sizeof(VertexPN) * index, &vertex, sizeof(VertexPN));
                }
            }
            else
            {
                VertexP vertex;
                vertex.position = glm::vec3((mesh.mVertices + index)->x, (mesh.mVertices + index)->y, (mesh.mVertices + index)->z);
                memcpy(vertexBufferDataPtr + sizeof(VertexP) * index, &vertex, sizeof(VertexP));
            }
        }
    }

    vertexBufferOnCPU.unmapCPUAccessibleDataPtr();

    void* indexBufferDataPtr = indexBufferOnCPU.getCPUAccessibleDataPtr();

    size_t indexBufferIndex = 0;
    for(auto currentFaceId = 0; currentFaceId < faceCount; ++currentFaceId)
    {
        const auto& face = *(mesh.mFaces + currentFaceId);
        for(auto faceIndexId = 0; faceIndexId < face.mNumIndices; ++faceIndexId)
        {
            memcpy(indexBufferDataPtr + sizeof(uint32_t) * indexBufferIndex, face.mIndices + faceIndexId, sizeof(uint32_t));
            ++indexBufferIndex;
        }
    }

    indexBufferOnCPU.unmapCPUAccessibleDataPtr();

    vertexBuffer.create(vertexCount * vertexSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, true, false);
    indexBuffer.create(indexCount * sizeof(uint32_t), vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, true, false);
    vertexBuffer.bindMemory();
    indexBuffer.bindMemory();

    vertexBuffer.updateDeviceLocal(bufferUpdateCBs[0], 
        vertexBufferOnCPU.getBuffer(),
        0,
        vk::Semaphore(),
        vk::Semaphore(),
        vk::Fence(),
        bufferUpdateFences[0],
        vk::PipelineStageFlagBits::eTopOfPipe,
        true);
    indexBuffer.updateDeviceLocal(bufferUpdateCBs[1],
        indexBufferOnCPU.getBuffer(),
        0,
        vk::Semaphore(),
        vk::Semaphore(),
        vk::Fence(),
        bufferUpdateFences[1],
        vk::PipelineStageFlagBits::eTopOfPipe,
        true);
    
    if(logicalDevice.waitForFences(2, bufferUpdateFences, true, ~0U) != vk::Result::eSuccess) throw std::runtime_error("Failed to wait for fences!\n");

    vertexBufferOnCPU.destroy();
    indexBufferOnCPU.destroy();
}

const Mesh& Mesh::bindVertexBuffer(spk::Subpass& subpass) const
{
    std::vector<vk::Buffer> vertexBuffers;
    vertexBuffers.push_back(vertexBuffer.getBuffer());
    subpass.bindVertexBuffers(vertexBuffers);

    return *this;
}

const Mesh& Mesh::bindIndexBuffer(spk::Subpass& subpass) const
{
    subpass.bindIndexBuffer(indexBuffer.getBuffer(), vk::IndexType::eUint32);

    return *this;
}

const Mesh& Mesh::bindPipeline(spk::Subpass& subpass, const uint32_t index) const
{
    subpass.bindPipeline(pipelines[index].getPipeline());

    return *this;
}

const Mesh& Mesh::bindDescriptorSets(spk::Subpass& subpass, const uint32_t pipelineIndex, const uint32_t firstSetToUpdate) const
{
    subpass.bindDescriptorSets(pipelines[pipelineIndex].getLayout(), descriptorSets, firstSetToUpdate);

    return *this;
}

const Mesh& Mesh::draw(spk::Subpass& subpass, const uint32_t instanceCount, const uint32_t firstInstance) const
{
    subpass.draw(vertexCount, instanceCount, 0, firstInstance);

    return *this;
}

const Mesh& Mesh::drawIndexed(spk::Subpass& subpass, const uint32_t instanceCount, const uint32_t firstInstance) const
{
    if(indexCount == 0) throw std::runtime_error("There are no indices to draw.\n");
    subpass.drawIndexed(indexCount, instanceCount, 0, firstInstance);

    return *this;
}

void Mesh::loadShaders(const uint32_t shaderSetIndex, const std::vector<spk::ShaderInfo> shaderInfos)
{
    if(shaderSetIndex >= shaderSets.size())
    {
        throw std::invalid_argument("Shader set index is out of range.\n");
    }
    shaderSets[shaderSetIndex].create(shaderInfos);
}

void Mesh::destroy()
{
    const auto& logicalDevice = spk::system::System::getInstance()->getLogicalDevice();
    const auto& pool = spk::system::Executives::getInstance()->getPool();

    if(bufferUpdateFences[0])
    {
        logicalDevice.destroyFence(bufferUpdateFences[0], nullptr);
        logicalDevice.destroyFence(bufferUpdateFences[1], nullptr);
        bufferUpdateFences[0] = bufferUpdateFences[1] = vk::Fence();
    }
    if(bufferUpdateCBs[0])
    {
        logicalDevice.freeCommandBuffers(pool, 2, bufferUpdateCBs);
    }
}
    
Mesh::~Mesh()
{
    destroy();
}