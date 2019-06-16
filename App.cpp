#include"App.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include"StbLibrary/stb_image.h"

void App::run()
{
    glfwSetInputMode(window.getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    const double fps = 60;
    const double frameTime = 1 / fps;
    auto startClock = std::chrono::system_clock::now();
    double timeCounter = 0;

    std::vector<spk::VertexBuffer*> pVertexBuffers = {&vertexBuffers[0]};
    
    while(!glfwWindowShouldClose(window.getGLFWWindow()))
    {
        glfwPollEvents();
        auto endClock = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = endClock - startClock;
        timeCounter += elapsed.count();
        startClock = endClock;

        processKeyboardInput(recalculateDirection(), static_cast<float>(elapsed.count()));

        if(timeCounter >= frameTime)
        {
            timeCounter -= frameTime;
            resourceSet.update(0, 0, &mvp);
            resourceSet.update(3, 0, &pos);
            window.draw(&resourceSet, &alignmentInfo, pVertexBuffers, &shaderSet);
        }
    }
}

const glm::vec3 App::recalculateDirection()
{
    static double lastX = WIDTH / 2;
    static double lastY = HEIGHT / 2;
    static double yaw = 0;
    static double pitch = 0;
    double cursorX, cursorY;
    glfwGetCursorPos(window.getGLFWWindow(), &cursorX, &cursorY);
    double offsetX = cursorX - lastX, offsetY = cursorY - lastY;
    lastX = cursorX;
    lastY = cursorY;
    double sensitivity = 0.05;
    offsetX *= sensitivity;
    offsetY *= sensitivity;
    yaw += offsetX;
    pitch += offsetY;
    if(pitch < -89.0) pitch = -89.0;
    if(pitch > 89.0) pitch = 89.0;
    glm::vec3 direction;
    direction.x = glm::cos(glm::radians(pitch)) * glm::cos(glm::radians(yaw));
    direction.y = -glm::cos(glm::radians(pitch)) * glm::sin(glm::radians(yaw));
    direction.z = -glm::sin(glm::radians(pitch));
    return glm::normalize(direction);
}

void App::processKeyboardInput(const glm::vec3 direction, const float elapsedTime)
{
    static const glm::vec3 up(0, 0, 1);
    int WState = glfwGetKey(window.getGLFWWindow(), GLFW_KEY_W);
    int AState = glfwGetKey(window.getGLFWWindow(), GLFW_KEY_A);
    int SState = glfwGetKey(window.getGLFWWindow(), GLFW_KEY_S);
    int DState = glfwGetKey(window.getGLFWWindow(), GLFW_KEY_D);
    if(WState == GLFW_PRESS)
    {
        pos += direction * elapsedTime;
    }
    if(SState == GLFW_PRESS)
    {
        pos -= direction * elapsedTime;
    }
    if(AState == GLFW_PRESS)
    {
        pos += glm::normalize(glm::cross(up, direction)) * elapsedTime;
    }
    if(DState == GLFW_PRESS)
    {
        pos -= glm::normalize(glm::cross(up, direction)) * elapsedTime;
    }
    mvp.view = glm::lookAt(pos, pos + direction, up);
}

App::App(): pos(-2, 2, 0), vertexBuffers(1)
{
    loadSceneData();
    cube.instances = {{0, 0, 0, 0}, {2, 2, 2, 0}};

    spk::DrawOptions opts;
    opts.cullMode = spk::CullMode::Clockwise;
    window.create(WIDTH, HEIGHT, "App", opts);

    mvp.model = glm::mat4(1);
    mvp.view = glm::lookAt(pos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    mvp.proj = glm::perspective(glm::radians<float>(60), WIDTH / (float)HEIGHT, 0.1f, 10.f);
    mvp.proj[1][1] = -mvp.proj[1][1];

    std::vector<spk::ShaderInfo> infos(2);
    infos[0].type = spk::ShaderType::Vertex;
    infos[0].filename = "vert.spv";
    infos[1].type = spk::ShaderType::Fragment;
    infos[1].filename = "frag.spv";
    shaderSet.create(infos);
    
    createVertexBuffer();
    createResourceSet();
    vertexBuffers[0].updateVertexBuffer(cube.positions.data(), 0);
    vertexBuffers[0].updateVertexBuffer(cube.uvs.data(), 1);
    vertexBuffers[0].updateVertexBuffer(cube.normals.data(), 2);
    vertexBuffers[0].updateIndexBuffer(cube.indices.data());
}

void App::loadSceneData()
{
    const std::string filename = "DonutWithStoneTexture.obj";
    const aiScene* scene = importer.ReadFile(filename, aiProcess_JoinIdenticalVertices);
    if(!scene) throw std::runtime_error("Failed to create scene!\n");

    getSceneMaterials(scene, sceneMaterials);
    initMeshFromScene(scene, 1, cube);
    vertexCount = cube.positions.size();
}

void App::initMeshFromScene(const aiScene* scene, const uint32_t meshIndex, Mesh& mesh) const
{
    aiMesh* currentMesh = *(scene->mMeshes + meshIndex);
    getMeshIndexData(currentMesh, mesh.indices);
    getMeshVertexData(currentMesh, mesh.positions);
    getMeshUVData(currentMesh, mesh.uvs);
    getMeshNormalData(currentMesh, mesh.normals);
}

void App::getMeshVertexData(aiMesh* mesh, std::vector<glm::vec3>& data) const 
{
    const unsigned int vertexCount = mesh->mNumVertices;
    data.resize(vertexCount);
    aiVector3D* vertex = mesh->mVertices;
    for(int i = 0; i < vertexCount; ++i)
    {
        data[i] = {vertex->x, vertex->y, vertex->z};
        ++vertex;
    }
}

void App::getMeshNormalData(aiMesh* mesh, std::vector<glm::vec3>& data) const
{
    const unsigned int vertexCount = mesh->mNumVertices;
    data.resize(vertexCount);
    aiVector3D* normal = mesh->mNormals;
    for(int i = 0; i < vertexCount; ++i)
    {
        data[i] = {normal->x, normal->y, normal->z};
        ++normal;
    }
}

void App::getMeshUVData(aiMesh* mesh, std::vector<glm::vec2>& data) const
{
    const unsigned int vertexCount = mesh->mNumVertices;
    data.resize(vertexCount);
    aiVector3D* uv = mesh->mTextureCoords[0];
    for(int i = 0; i < vertexCount; ++i)
    {
        data[i] = {uv->x, uv->y};
        ++uv;
    }
}

void App::getMeshIndexData(aiMesh* mesh, std::vector<uint32_t>& data) const
{
    const unsigned int faceCount = mesh->mNumFaces;
    data = std::vector<uint32_t>();
    aiFace* face = mesh->mFaces;
    for(int i = 0; i < faceCount; ++i)
    {
        unsigned int* index = face->mIndices;
        for(int j = 0; j < face->mNumIndices; ++j)
        {
            data.push_back(static_cast<uint32_t>(*index));
            ++index;
        }
        ++face;
    }
}

void App::getSceneMaterials(const aiScene* scene, std::vector<aiMaterial*>& materials) const
{
    const unsigned int materialCount = scene->mNumMaterials;
    materials.resize(materialCount);
    aiMaterial** material = scene->mMaterials;
    for(int i = 0; i < materialCount; ++i)
    {
        materials[i] = (*material);
        ++material;
    }
}

const aiMaterial* App::getMeshMaterial(aiMesh* mesh, const std::vector<aiMaterial*>& materials) const
{
    return materials[mesh->mMaterialIndex];
}

void App::createVertexBuffer()
{
    spk::BindingAlignmentInfo positionInfo;
    positionInfo.binding = 0;
    positionInfo.structSize = sizeof(glm::vec3);
    positionInfo.fields = std::vector<spk::StructFieldInfo>(1);
    positionInfo.fields[0] = {0, spk::FieldFormat::vec3f, 0};

    spk::BindingAlignmentInfo uvInfo;
    uvInfo.binding = 1;
    uvInfo.structSize = sizeof(glm::vec2);
    uvInfo.fields = std::vector<spk::StructFieldInfo>(1);
    uvInfo.fields[0] = {1, spk::FieldFormat::vec2f, 0};

    spk::BindingAlignmentInfo normalInfo;
    normalInfo.binding = 2;
    normalInfo.structSize = sizeof(glm::vec3);
    normalInfo.fields = std::vector<spk::StructFieldInfo>(1);
    normalInfo.fields[0] = {2, spk::FieldFormat::vec3f, 0};

    std::vector<spk::BindingAlignmentInfo> ais = {positionInfo, uvInfo, normalInfo};

    alignmentInfo.create(ais);

    std::vector<uint32_t> sizes = {
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec3)), 
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec2)), 
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec3))
    };

    std::vector<uint32_t> bindings = {0, 1, 2};

    vertexBuffers[0].create(bindings, sizes, cube.indices.size() * sizeof(uint32_t));
    vertexBuffers[0].setInstancingOptions(cube.instances.size(), 0);
}

void App::createResourceSet()
{
    int width, height, channels;

    textureData = stbi_load("textureWALL.jpg", &width, &height, &channels, 4);
    textureWALL.create(width, height, spk::ImageFormat::RGBA8, 2, 0);

    uniformBuffer.create(sizeof(MVP), 0, 0);
    instancingBuffer.create(sizeof(glm::vec4) * cube.instances.size(), 1, 0);

    spk::UniformBuffer cameraPos;
    cameraPos.create(sizeof(glm::vec3), 3, 0);

    std::vector<spk::Texture> textures;
    textures.push_back(textureWALL);

    std::vector<spk::UniformBuffer> ubs;
    ubs.push_back(uniformBuffer);
    ubs.push_back(instancingBuffer);
    ubs.push_back(cameraPos);

    resourceSet.create(textures, ubs);
    resourceSet.update(0, 0, &mvp);
    resourceSet.update(1, 0, cube.instances.data());
    resourceSet.update(2, 0, reinterpret_cast<const void *>(textureData));
    resourceSet.update(3, 0, &pos);

}

App::~App()
{
    delete textureData;
}