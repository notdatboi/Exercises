#ifndef TEST_APP_HPP
#define TEST_APP_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include"../Spark/Window.hpp"
#include<fstream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<chrono>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

struct Vertex
{
    glm::vec2 position;
    glm::vec2 uv;
};

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class App
{
public:
    App();
    void run();
    ~App();
private:
    void createResourceSet();
    void createVertexBuffer();
    const glm::vec3 recalculateDirection();
    void processKeyboardInput(const glm::vec3 direction, const float elapsedTime);
    void loadSceneData();
    void getMeshVertexData(aiMesh* mesh, std::vector<glm::vec3>& data) const;
    void getMeshNormalData(aiMesh* mesh, std::vector<glm::vec3>& data) const;
    void getMeshUVData(aiMesh* mesh, std::vector<glm::vec2>& data) const;
    void getMeshIndexData(aiMesh* mesh, std::vector<uint32_t>& data) const;
    void getSceneMaterials(aiScene* scene, std::vector<aiMaterial>& materials) const;
    const aiMaterial& getMaterial(aiMesh* mesh, const std::vector<aiMaterial>& materials) const;
    
    const uint32_t WIDTH = 1024;
    const uint32_t HEIGHT = 720;

    MVP mvp;
    unsigned int vertexCount;
    Assimp::Importer importer;
    unsigned char* textureData;

    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<uint32_t> indices;
    std::vector<glm::vec4> instances;

    spk::VertexBuffer vertexBuffer;
    spk::Window window;
    spk::ResourceSet resourceSet;
    spk::Texture textureWALL;
    spk::UniformBuffer uniformBuffer;
    spk::UniformBuffer instancingBuffer;
    spk::ShaderSet shaderSet;
    glm::vec3 pos;
};

#endif