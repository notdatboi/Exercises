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
            window.draw(&resourceSet, &vertexBuffer, &shaderSet);
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

App::App(): pos(3, 0, 0), instances({{0, 0, 0, 0}, {2, 2, 2, 0}})
{
    loadSceneData();

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
    vertexBuffer.updateVertexBuffer(positions.data(), 0);
    vertexBuffer.updateVertexBuffer(uvs.data(), 1);
    vertexBuffer.updateVertexBuffer(normals.data(), 2);
    vertexBuffer.updateIndexBuffer(indices.data());
}

void App::loadSceneData()
{
    const std::string filename = "Model.obj";
    const aiScene* scene = importer.ReadFile(filename, aiProcess_JoinIdenticalVertices);
    if(!scene) throw std::runtime_error("Failed to create scene!\n");

    aiMesh* mesh = (*scene->mMeshes);
    vertexCount = (*scene->mMeshes)->mNumVertices;
    unsigned int faceCount = (*scene->mMeshes)->mNumFaces;

    for(int i = 0; i < faceCount; ++i)
    {
        aiFace x = *((*scene->mMeshes)->mFaces + i);
        for(int j = 0; j < x.mNumIndices; ++j)
        {
            indices.push_back(*(x.mIndices + j));
        }
    }

    for(int i = 0; i < vertexCount; ++i)
    {
        uvs.push_back({(mesh->mTextureCoords[0] + i)->x, (mesh->mTextureCoords[0] + i)->y});
        positions.push_back({(mesh->mVertices + i)->x, (mesh->mVertices + i)->y, (mesh->mVertices + i)->z});
        normals.push_back({(mesh->mNormals + i)->x, (mesh->mNormals + i)->y, (mesh->mNormals + i)->z});
    }
}

void App::createVertexBuffer()
{
    spk::VertexAlignmentInfo positionInfo;
    positionInfo.binding = 0;
    positionInfo.structSize = sizeof(glm::vec3);
    positionInfo.fields = std::vector<spk::StructFieldInfo>(1);
    positionInfo.fields[0] = {0, spk::FieldFormat::vec3f, 0};

    spk::VertexAlignmentInfo uvInfo;
    uvInfo.binding = 1;
    uvInfo.structSize = sizeof(glm::vec2);
    uvInfo.fields = std::vector<spk::StructFieldInfo>(1);
    uvInfo.fields[0] = {1, spk::FieldFormat::vec2f, 0};

    spk::VertexAlignmentInfo normalInfo;
    normalInfo.binding = 2;
    normalInfo.structSize = sizeof(glm::vec3);
    normalInfo.fields = std::vector<spk::StructFieldInfo>(1);
    normalInfo.fields[0] = {2, spk::FieldFormat::vec3f, 0};

    std::vector<spk::VertexAlignmentInfo> ais = {positionInfo, uvInfo, normalInfo};
    std::vector<uint32_t> sizes = {
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec3)), 
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec2)), 
        vertexCount * static_cast<uint32_t>(sizeof(glm::vec3))
    };
    vertexBuffer.create(ais, sizes, indices.size() * sizeof(uint32_t));
    vertexBuffer.setInstancingOptions(2, 0);
}

void App::createResourceSet()
{
    int width, height, channels;

    textureData = stbi_load("textureWALL.jpg", &width, &height, &channels, 4);
    textureWALL.create(width, height, spk::ImageFormat::RGBA8, 2, 0);

    uniformBuffer.create(sizeof(MVP), 0, 0);
    instancingBuffer.create(sizeof(glm::vec4) * instances.size(), 1, 0);

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
    resourceSet.update(1, 0, instances.data());
    resourceSet.update(2, 0, reinterpret_cast<const void *>(textureData));
    resourceSet.update(3, 0, &pos);

}

App::~App()
{
    delete textureData;
}