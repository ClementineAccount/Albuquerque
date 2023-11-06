#include <PlaygroundApplication.hpp>
#include <SandboxApplication.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


static constexpr bool isFwog = false-;

int main(int argc, char* argv[])
{
    if (!isFwog)
    {
        SandboxApplication application;
        application.Run();
    }
    else
    {
        PlaygroundApplication application;
        application.Run();
    }

    return 0;
}