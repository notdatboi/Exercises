#include"App.hpp"

int main()
{
    spk::system::init();
    App* app = new App();
    app->run();
    delete app;
    spk::system::deinit();
}