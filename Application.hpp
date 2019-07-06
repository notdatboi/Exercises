#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include"Mesh.hpp"
//#include<map>
//#include<string>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
//#include<Spark.hpp>
#include"TextureHolder.hpp"
#include"Camera.hpp"
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

struct MVP
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Application
{
public:
    Application();
    void run();
    ~Application();
private:
    void createWindowAndSurface();
    void createSwapchain();
    void loadDescriptorSets();
    void loadAndWriteDonutTexture();
    //void updateDescriptorSets();
    void loadShaders();
    //void createDepthPrepass();
    void createGBufferPass();
    void createRenderPass();        // and subpass dependency (l8r)
    void createGPassPipeline();
    void createFramebuffers();
    void loadDonutMesh(const std::string donutFilename);
    void recordRenderPass();

    void getMeshVertexData(const aiMesh* mesh, std::vector<Vertex>& vertices) const;
    const std::vector<Vertex> getMeshVertexData(const aiMesh* mesh) const;
    void getMeshIndexData(const aiMesh* mesh, std::vector<uint32_t>& indices) const;
    const std::vector<uint32_t> getMeshIndexData(const aiMesh* mesh) const;

    void processInput();
    void createDescriptorBuffers();
    void updateMVPDescriptorSet();
    void updateCameraDescriptorSet();
    void updateDonutInstancesDescriptorSet();

    const unsigned int windowWidth = 1024;
    const unsigned int windowHeight = 720;

    MVP mvp;
    Camera camera;
    std::vector<glm::vec4> donutInstances;
    spk::Buffer mvpBuffer;
    spk::Buffer cameraBuffer;
    spk::Buffer donutInstancesBuffer;
    const uint32_t mvpSetIndex = 0;
    const uint32_t cameraSetIndex = 1;
    const uint32_t donutInstancesSetIndex = 2;
    const uint32_t textureSetIndex = 3;

    const uint32_t gBufferPassID = 0;
    const vk::Format swapchainImageFormat = vk::Format::eR8G8B8A8Snorm;
    
    GLFWwindow* window;
    vk::SurfaceKHR surface;
    spk::Swapchain swapchain;
    TextureHolder textureHolder;
    spk::DescriptorPool descriptorPool;
    //spk::ShaderSet depthPrepassShaders;
    spk::ShaderSet gPassShaders;
    spk::Pipeline gPassPipeline;
    //std::vector<spk::Image> depthMaps;
    //spk::Subpass depthPrepass;
    spk::Subpass gBufferPass;
    //vk::SubpassDependency depthToGBufferDependency;
    spk::RenderPass renderPass;
    spk::Image donutTexture;
    spk::ImageView donutTextureView;
    Mesh donut;
};

#endif