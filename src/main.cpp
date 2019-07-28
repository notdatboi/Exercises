#include"../include/Application.hpp"

int main()
{
    glfwInit();
    uint32_t count = 0;
    glfwGetRequiredInstanceExtensions(&count);
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> glfwExtensionsVector(glfwExtensions, glfwExtensions + count);
    spk::system::init(glfwExtensionsVector);
    Application* app = new Application();
    app->run();
    delete app;
    spk::system::deinit();
    glfwTerminate();
}