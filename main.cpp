#include"Application.hpp"

int main()
{
    glfwInit();
    std::vector<const char*> glfwExtensions;
    uint32_t count = 0;
    glfwGetRequiredInstanceExtensions(&count);
    glfwExtensions.resize(count);
    glfwExtensions.data = glfwGetRequiredInstanceExtensions(&count);
    spk::system::init(glfwExtensions);
    Application* app = new Application();
    app->run();
    delete app;
    spk::system::deinit();
    glfwTerminate();
}